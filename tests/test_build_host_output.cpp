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
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
                   "build host DLL wrapper should declare manifest flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
                   "build host DLL wrapper should route manifest flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
                   "build host DLL wrapper should declare library-export flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
                   "build host DLL wrapper should route library-export flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
                   "build host DLL wrapper should declare routine-kind flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
                   "build host DLL wrapper should route routine-kind flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
                   "build host DLL wrapper should declare source-path flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
                   "build host DLL wrapper should route source-path flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
                   "build host DLL wrapper should declare source-line flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
                   "build host DLL wrapper should route source-line flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
                   "build host DLL wrapper should declare parameter-declaration flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
                   "build host DLL wrapper should route parameter-declaration flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
                   "build host DLL wrapper should declare parameter-names flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
                   "build host DLL wrapper should route parameter-names flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
                   "build host DLL wrapper should declare parameter-count flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
                   "build host DLL wrapper should route parameter-count flag through helper");
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
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturn copperfin_build_runtime_bridge_placeholder_return_binding(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder return-binding helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeEnvironmentVariable") != std::string::npos,
                   "build host DLL wrapper should declare a launch-environment surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeLaunchPlan") != std::string::npos,
                   "build host DLL wrapper should declare a launch-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeLaunchPlan copperfin_build_runtime_bridge_launch_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a launch-plan helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
                   "build host DLL wrapper should declare library-export env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
                   "build host DLL wrapper should route library-export env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
                   "build host DLL wrapper should declare routine-kind env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
                   "build host DLL wrapper should route routine-kind env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
                   "build host DLL wrapper should declare source-path env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
                   "build host DLL wrapper should route source-path env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
                   "build host DLL wrapper should declare parameter-count env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
                   "build host DLL wrapper should route parameter-count env-var through helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeObservationPlan") != std::string::npos,
                   "build host DLL wrapper should declare an observation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeObservationPlan copperfin_build_runtime_bridge_observation_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an observation-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeExecutionPlan") != std::string::npos,
                   "build host DLL wrapper should declare an execution-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeExecutionPlan copperfin_build_runtime_bridge_execution_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an execution-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeTransportPlan") != std::string::npos,
                   "build host DLL wrapper should declare a transport-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeTransportPlan copperfin_build_runtime_bridge_transport_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a transport-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeSerializationPlan") != std::string::npos,
                   "build host DLL wrapper should declare a serialization-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeSerializationPlan copperfin_build_runtime_bridge_serialization_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a serialization-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeDispatchPlan") != std::string::npos,
                   "build host DLL wrapper should declare a dispatch-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchPlan copperfin_build_runtime_bridge_dispatch_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a dispatch-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchExecution copperfin_runtime_bridge_execute_dispatch(") != std::string::npos,
                   "build host DLL wrapper should declare a shared dispatch-execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_launch_process(") != std::string::npos,
                   "build host DLL wrapper should declare a shared process-launch helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeHostFailureEvaluation copperfin_runtime_bridge_evaluate_host_failure(") != std::string::npos,
                   "build host DLL wrapper should declare a shared host-failure evaluation helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeMissingResponseEvaluation copperfin_runtime_bridge_evaluate_missing_response(") != std::string::npos,
                   "build host DLL wrapper should declare a shared missing-response evaluation helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationEvaluation copperfin_runtime_bridge_evaluate_response_validation(") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-validation evaluation helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgePayloadPlan") != std::string::npos,
                   "build host DLL wrapper should declare a payload-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePayloadPlan copperfin_build_runtime_bridge_payload_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a payload-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretationPlan") != std::string::npos,
                   "build host DLL wrapper should declare an interpretation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretationPlan copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an interpretation-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeFailurePolicyPlan") != std::string::npos,
                   "build host DLL wrapper should declare a failure-policy surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFailurePolicyPlan copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a failure-policy helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-status field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-value field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-diagnostics field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request payload-shape helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response payload-shape helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared export-name field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-count field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameters field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request-media-type field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-media-type field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared payload-shape field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
                   "build host DLL wrapper should route payload-shape field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-name field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
                   "build host DLL wrapper should route parameter-name field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-value field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
                   "build host DLL wrapper should route parameter-value field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-surface field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
                   "build host DLL wrapper should route parameter-surface field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared failure-diagnostics token helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared success-status token helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseValidationPlan") != std::string::npos,
                   "build host DLL wrapper should declare a response-validation surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationPlan copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a response-validation helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestArtifact") != std::string::npos,
                   "build host DLL wrapper should declare a request-artifact surface");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_document(") != std::string::npos,
                   "build host DLL wrapper should declare a request-document helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestArtifact copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
                   "build host DLL wrapper should declare a request-artifact helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestWritePlan") != std::string::npos,
                   "build host DLL wrapper should declare a request-write-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestWritePlan copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a request-write-plan helper");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_execute_write_request(") != std::string::npos,
                   "build host DLL wrapper should declare a shared request-write execution helper.");
            expect(wrapper_source.find("out << plan.request_artifact.request_document;") != std::string::npos,
                   "build host DLL wrapper should stage request-document writes through the shared request-write execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseReadPlan") != std::string::npos,
                   "build host DLL wrapper should declare a response-read-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseReadPlan copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a response-read-plan helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_execute_read_response(") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-read execution helper.");
            expect(wrapper_source.find("response_document << input.rdbuf();") != std::string::npos,
                   "build host DLL wrapper should stage response-document reads through the shared response-read execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseArtifact") != std::string::npos,
                   "build host DLL wrapper should declare a response-artifact surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseArtifact copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
                   "build host DLL wrapper should declare a response-artifact helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseParsePlan") != std::string::npos,
                   "build host DLL wrapper should declare a response-parse-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParsePlan copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a response-parse-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParseAdmission copperfin_runtime_bridge_admit_response_parse(") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-parse admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeParsedResponse copperfin_runtime_bridge_execute_parse_response(") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-parse execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(response_document, plan.status_field)") != std::string::npos,
                   "build host DLL wrapper should stage response field extraction through the shared response-parse execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretedResultPlan") != std::string::npos,
                   "build host DLL wrapper should declare an interpreted-result-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultPlan copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an interpreted-result-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultAdmission copperfin_runtime_bridge_admit_interpreted_result(") != std::string::npos,
                   "build host DLL wrapper should declare a shared interpreted-result admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResult copperfin_runtime_bridge_execute_interpreted_result(") != std::string::npos,
                   "build host DLL wrapper should declare a shared interpreted-result execution helper.");
            expect(wrapper_source.find("parsed_response.status_value == plan.success_status_value") != std::string::npos,
                   "build host DLL wrapper should stage interpreted-result selection through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeNativeReturnPlan") != std::string::npos,
                   "build host DLL wrapper should declare a native-return-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnPlan copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a native-return-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnAdmission copperfin_runtime_bridge_admit_native_return(") != std::string::npos,
                   "build host DLL wrapper should declare a shared native-return admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturn copperfin_runtime_bridge_execute_native_return(") != std::string::npos,
                   "build host DLL wrapper should declare a shared native-return execution helper.");
            expect(wrapper_source.find("interpreted_result.matched_success_status") != std::string::npos,
                   "build host DLL wrapper should stage native-return selection through the shared execution helper.");
            expect(wrapper_source.find("static int copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host DLL wrapper should declare an integer return-representation parser");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_default_int_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parsed-int default sentinel helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_default_int_value()") != std::string::npos,
                   "build host DLL wrapper should route the parsed-int default sentinel through the shared helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeOutcomeSelectionPlan") != std::string::npos,
                   "build host DLL wrapper should declare an outcome-selection-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionPlan copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an outcome-selection-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionAdmission copperfin_runtime_bridge_admit_outcome_selection(") != std::string::npos,
                   "build host DLL wrapper should declare a shared outcome-selection admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelection copperfin_runtime_bridge_execute_outcome_selection(") != std::string::npos,
                   "build host DLL wrapper should declare a shared outcome-selection execution helper.");
            expect(wrapper_source.find("native_return.matched_success_status") != std::string::npos,
                   "build host DLL wrapper should stage outcome selection through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnMaterializationPlan") != std::string::npos,
                   "build host DLL wrapper should declare a return-materialization-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationPlan copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a return-materialization-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationAdmission copperfin_runtime_bridge_admit_return_materialization(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-materialization admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterialization copperfin_runtime_bridge_execute_return_materialization(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-materialization execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_outcome_selection(plan.outcome_selection_plan)") != std::string::npos,
                   "build host DLL wrapper should stage return materialization through the shared execution helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
                   "build host DLL wrapper should declare a shared native-int return-surface helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
                   "build host DLL wrapper should route native-int return-surface comparisons through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_placeholder_signature_token()") != std::string::npos,
                   "build host DLL wrapper should declare a shared native-int placeholder-signature helper.");
            expect(wrapper_source.find("find(copperfin_build_runtime_bridge_native_int_placeholder_signature_token())") != std::string::npos,
                   "build host DLL wrapper should route native-int placeholder-signature matching through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
                   "build host DLL wrapper should declare a shared native return-statement framing helper.");
            expect(wrapper_source.find("return copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
                   "build host DLL wrapper should route native return-statement framing through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
                   "build host DLL wrapper should declare a shared typed native return-expression helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
                   "build host DLL wrapper should route typed native return-expression construction through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should declare a shared stdout log-file suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should route stdout log-file suffix through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should declare a shared stderr log-file suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should route stderr log-file suffix through the shared helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
                   "build host DLL wrapper should declare a shared expected-exit-code helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
                   "build host DLL wrapper should route expected-exit-code through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request artifact suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should route request artifact suffix through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response artifact suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should route response artifact suffix through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared activates-adopted-return policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
                   "build host DLL wrapper should route activates-adopted-return policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared capture-stdout policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
                   "build host DLL wrapper should route capture-stdout policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared capture-stderr policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
                   "build host DLL wrapper should route capture-stderr policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared fail-on-nonzero-exit policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
                   "build host DLL wrapper should route fail-on-nonzero-exit policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared fail-on-missing-response policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
                   "build host DLL wrapper should route fail-on-missing-response policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared ensure-parent-directory policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
                   "build host DLL wrapper should route ensure-parent-directory policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared require-existing-response policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
                   "build host DLL wrapper should route require-existing-response policy through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
                   "build host DLL wrapper should declare a shared replace-placeholder-return adoption-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
                   "build host DLL wrapper should route replace-placeholder-return mode token through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
                   "build host DLL wrapper should declare a shared planned-activation-pending activation-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
                   "build host DLL wrapper should route planned-activation-pending mode token through the shared helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnEmissionPlan") != std::string::npos,
                   "build host DLL wrapper should declare a return-emission-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionPlan copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a return-emission-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionAdmission copperfin_runtime_bridge_admit_return_emission(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-emission admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmission copperfin_runtime_bridge_execute_return_emission(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-emission execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_return_materialization(plan.return_materialization_plan)") != std::string::npos,
                   "build host DLL wrapper should stage return emission through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeFinalReturnAdoptionPlan") != std::string::npos,
                   "build host DLL wrapper should declare a final-return-adoption-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionPlan copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a final-return-adoption-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionAdmission copperfin_runtime_bridge_admit_final_return_adoption(") != std::string::npos,
                   "build host DLL wrapper should declare a shared final-return-adoption admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoption copperfin_runtime_bridge_execute_final_return_adoption(") != std::string::npos,
                   "build host DLL wrapper should declare a shared final-return-adoption execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_return_emission(plan.return_emission_plan)") != std::string::npos,
                   "build host DLL wrapper should stage final-return adoption through the shared execution helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_placeholder_return_statement(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder return-statement helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnActivationPlan") != std::string::npos,
                   "build host DLL wrapper should declare a return-activation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationPlan copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a return-activation-plan helper");
            expect(wrapper_source.find("CopperfinRuntimeBridgeStubEmissionWrapper stub_emission_wrapper;") != std::string::npos,
                   "build host DLL wrapper should carry the stub-emission wrapper contract through the descriptor plan.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationAdmission copperfin_runtime_bridge_admit_return_activation(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-activation admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivation copperfin_runtime_bridge_execute_return_activation(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-activation execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_final_return_adoption(plan.final_return_adoption_plan)") != std::string::npos,
                   "build host DLL wrapper should stage return activation through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubReturnPlan") != std::string::npos,
                   "build host DLL wrapper should declare a stub-return-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnPlan copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a stub-return-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnAdmission copperfin_runtime_bridge_admit_stub_return(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-return admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturn copperfin_runtime_bridge_execute_stub_return(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-return execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_return_activation(plan.return_activation_plan)") != std::string::npos,
                   "build host DLL wrapper should stage stub-return handling through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgePlaceholderReturnValuePlan") != std::string::npos,
                   "build host DLL wrapper should declare a placeholder-return-value-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValuePlan copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a placeholder-return-value-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValueAdmission copperfin_runtime_bridge_admit_placeholder_return_value(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder-return-value admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValue copperfin_runtime_bridge_execute_placeholder_return_value(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder-return-value execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_stub_return(plan.stub_return_plan)") != std::string::npos,
                   "build host DLL wrapper should stage placeholder-return-value handling through the shared execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnIntAdmission copperfin_runtime_bridge_admit_placeholder_return_int(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder-return-int admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmission copperfin_runtime_bridge_execute_stub_emission(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionReturnSurface copperfin_runtime_bridge_build_stub_emission_return_surface(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission return-surface helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_apply_stub_emission_output(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission output-application helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubEmissionWrapper") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission wrapper surface.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionWrapper copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission wrapper helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return_shared(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission emitter helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_execute_placeholder_return_int(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder-return-int execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_value)};") != std::string::npos,
                   "build host DLL wrapper should route placeholder-return-int execution through the shared stub-emission helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_return_native_int(int value)") != std::string::npos,
                   "build host DLL wrapper should declare the DLL native-int return adapter for shared output application.");
            expect(wrapper_source.find("const auto stub_emission_wrapper =\n        copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
                   "build host DLL wrapper should build a shared stub-emission wrapper before building the descriptor plan.");
            expect(wrapper_source.find("return copperfin_runtime_bridge_emit_stub_return_shared(") != std::string::npos,
                   "build host DLL wrapper should route DLL stub emission through the shared emitter helper.");
            expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.native_return_surface") != std::string::npos,
                   "build host DLL wrapper should read the stub-emission return surface through the descriptor plan.");
            expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter") != std::string::npos,
                   "build host DLL wrapper should read the stub-emission return adapter through the descriptor plan.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_native_int_return_surface(),") != std::string::npos,
                   "build host DLL wrapper should pass the DLL native int return-surface contract into the shared wrapper helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_return_native_int);") != std::string::npos,
                   "build host DLL wrapper should pass the DLL native-int return adapter into the shared wrapper helper.");
            expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
                   "build host DLL wrapper should pass the built wrapper into the descriptor-plan builder.");
            expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n        interpretation_plan,\n        placeholder_return_binding.value_representation);") != std::string::npos,
                   "build host DLL wrapper should build the failure-policy plan from the enriched interpretation plan.");
            expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n        failure_policy);") != std::string::npos,
                   "build host DLL wrapper should build the response-validation plan from the enriched failure-policy plan.");
            expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n        response_validation);") != std::string::npos,
                   "build host DLL wrapper should build the request artifact directly from the response-validation plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n        request_artifact);") != std::string::npos,
                   "build host DLL wrapper should build the request-write plan directly from the request artifact once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n        request_write_plan);") != std::string::npos,
                   "build host DLL wrapper should build the response-read plan directly from the request-write plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto response_document =\n        copperfin_runtime_bridge_execute_read_response(response_read_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the response-read plan before building the response artifact.");
            expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n        response_read_plan,\n        response_document);") != std::string::npos,
                   "build host DLL wrapper should build the response artifact from the response-read plan and executed response document.");
            expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n        response_artifact);") != std::string::npos,
                   "build host DLL wrapper should build the response-parse plan directly from the response artifact once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto parsed_response =\n        copperfin_runtime_bridge_execute_parse_response(response_parse_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the response-parse plan before building the interpreted-result plan.");
            expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n        response_parse_plan,\n        parsed_response);") != std::string::npos,
                   "build host DLL wrapper should build the interpreted-result plan from the response-parse plan and parsed response.");
            expect(wrapper_source.find("const auto interpreted_result =\n        copperfin_runtime_bridge_execute_interpreted_result(interpreted_result_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the interpreted-result plan before building the native-return plan.");
            expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n        result,\n        interpreted_result_plan,\n        interpreted_result);") != std::string::npos,
                   "build host DLL wrapper should build the native-return plan from the interpreted-result plan and interpreted result.");
            expect(wrapper_source.find("const auto native_return =\n        copperfin_runtime_bridge_execute_native_return(native_return_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the native-return plan before building the outcome-selection plan.");
            expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n        native_return_plan,\n        native_return);") != std::string::npos,
                   "build host DLL wrapper should build the outcome-selection plan from the native-return plan and native return.");
            expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n        outcome_selection_plan);") != std::string::npos,
                   "build host DLL wrapper should build the return-materialization plan directly from the outcome-selection plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n        return_materialization_plan);") != std::string::npos,
                   "build host DLL wrapper should build the return-emission plan directly from the return-materialization plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n        return_emission_plan,\n        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));") != std::string::npos,
                   "build host DLL wrapper should build the final-return-adoption plan directly from the return-emission plan once the wrapper contract is carried by response validation.");
            expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n        final_return_adoption_plan);") != std::string::npos,
                   "build host DLL wrapper should build the return-activation plan directly from the final-return-adoption plan once the wrapper contract is carried by response validation.");
            expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n        return_activation_plan);") != std::string::npos,
                   "build host DLL wrapper should build the stub-return plan directly from the return-activation plan once the wrapper contract is carried by response validation.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return(\n") == std::string::npos,
                   "build host DLL wrapper should no longer declare an output-specific stub-return wrapper helper once the plan carries the wrapper contract.");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"InitLibrary\"") != std::string::npos,
                   "build host DLL wrapper should build a bridge descriptor for InitLibrary");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"AddNumbers\"") != std::string::npos,
                   "build host DLL wrapper should build a bridge descriptor for AddNumbers");
            expect(wrapper_source.find("const auto invocation = copperfin_build_runtime_bridge_invocation(\n        descriptor);") != std::string::npos,
                   "build host DLL wrapper should build a bridge invocation from the descriptor");
            expect(wrapper_source.find("const auto call = copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host DLL wrapper should build a bridge call from the invocation");
            expect(wrapper_source.find("const auto result = copperfin_build_runtime_bridge_result(\n        call,\n        placeholder_return_binding);") != std::string::npos,
                   "build host DLL wrapper should build a bridge result from the enriched call");
            expect(wrapper_source.find("const auto placeholder_return_binding =") != std::string::npos,
                   "build host DLL wrapper should build a shared placeholder return binding before building the result");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_binding(\"int\")") != std::string::npos,
                   "build host DLL wrapper should build the DLL placeholder return binding through the shared helper");
            expect(wrapper_source.find("const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(\n        result);") != std::string::npos,
                   "build host DLL wrapper should build a launch plan from the result");
            expect(wrapper_source.find("const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(\n        launch_plan);") != std::string::npos,
                   "build host DLL wrapper should build an observation plan from the launch plan");
            expect(wrapper_source.find("const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(\n        observation_plan);") != std::string::npos,
                   "build host DLL wrapper should build an execution plan from the observation plan");
            expect(wrapper_source.find("const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(\n        execution_plan);") != std::string::npos,
                   "build host DLL wrapper should build a transport plan from the execution plan");
            expect(wrapper_source.find("const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(\n        transport_plan);") != std::string::npos,
                   "build host DLL wrapper should build a serialization plan from the transport plan");
            expect(wrapper_source.find("const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(\n        serialization_plan);") != std::string::npos,
                   "build host DLL wrapper should build a dispatch plan from the serialization plan");
            expect(wrapper_source.find("const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);") != std::string::npos,
                   "build host DLL wrapper should route the dispatch plan through the shared dispatch-execution helper.");
            expect(wrapper_source.find("(void)dispatch_execution;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only dispatch-execution result unused.");
            expect(wrapper_source.find("const auto process_launch = copperfin_runtime_bridge_launch_process(dispatch_execution);") != std::string::npos,
                   "build host DLL wrapper should route dispatch execution through the shared process-launch helper.");
            expect(wrapper_source.find("(void)process_launch;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only process-launch result unused.");
            expect(wrapper_source.find("const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(\n        dispatch_plan);") != std::string::npos,
                   "build host DLL wrapper should build a payload plan from the dispatch plan");
            expect(wrapper_source.find("const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
                   "build host DLL wrapper should build an interpretation plan from the payload plan");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_native_int_return_surface());") != std::string::npos,
                   "build host DLL wrapper stub should route wrapper-return-surface through native-int return-surface helper");
            expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
                   "build host DLL wrapper should build a failure policy from the interpretation plan");
            expect(wrapper_source.find("const auto host_failure =") != std::string::npos,
                   "build host DLL wrapper should evaluate staged host failure from the process-launch helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);") != std::string::npos,
                   "build host DLL wrapper should route process-launch output through the shared host-failure evaluation helper.");
            expect(wrapper_source.find("(void)host_failure;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only host-failure evaluation result unused.");
            expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
                   "build host DLL wrapper should build a response-validation plan from the failure policy");
            expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
                   "build host DLL wrapper should build a request artifact from the response validation plan");
            expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
                   "build host DLL wrapper should build a request write plan from the request artifact");
            expect(wrapper_source.find("const auto request_write_execution =\n        copperfin_runtime_bridge_execute_write_request(request_write_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the request-write plan through the shared helper.");
            expect(wrapper_source.find("(void)request_write_execution;") != std::string::npos,
                   "build host DLL wrapper should explicitly discard the scaffold-only request-write execution result.");
            expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
                   "build host DLL wrapper should build a response read plan from the request write plan");
            expect(wrapper_source.find("const auto missing_response =") != std::string::npos,
                   "build host DLL wrapper should evaluate staged missing-response policy from the host-failure and response-read helpers.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_missing_response(host_failure, response_read_plan);") != std::string::npos,
                   "build host DLL wrapper should route host-failure output through the shared missing-response evaluation helper.");
            expect(wrapper_source.find("(void)missing_response;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only missing-response evaluation result unused.");
            expect(wrapper_source.find("const auto response_validation_evaluation =") != std::string::npos,
                   "build host DLL wrapper should evaluate staged response-validation policy from the missing-response and validation helpers.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_response_validation(missing_response, response_validation);") != std::string::npos,
                   "build host DLL wrapper should route missing-response output through the shared response-validation evaluation helper.");
            expect(wrapper_source.find("(void)response_validation_evaluation;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only response-validation evaluation result unused.");
            expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
                   "build host DLL wrapper should build a response artifact from the response read plan");
            expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
                   "build host DLL wrapper should build a response parse plan from the response artifact");
            expect(wrapper_source.find("const auto response_parse_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged response parsing from the response-validation evaluation and parse plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);") != std::string::npos,
                   "build host DLL wrapper should route response-validation evaluation through the shared response-parse admission helper.");
            expect(wrapper_source.find("(void)response_parse_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only response-parse admission result unused.");
            expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
                   "build host DLL wrapper should build an interpreted result plan from the response parse plan");
            expect(wrapper_source.find("const auto interpreted_result_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged interpreted-result selection from the response-parse admission and interpreted-result plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);") != std::string::npos,
                   "build host DLL wrapper should route response-parse admission through the shared interpreted-result admission helper.");
            expect(wrapper_source.find("(void)interpreted_result_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only interpreted-result admission result unused.");
            expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
                   "build host DLL wrapper should build a native return plan from the interpreted result plan");
            expect(wrapper_source.find("const auto native_return_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged native-return selection from the interpreted-result admission and native-return plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);") != std::string::npos,
                   "build host DLL wrapper should route interpreted-result admission through the shared native-return admission helper.");
            expect(wrapper_source.find("(void)native_return_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only native-return admission result unused.");
            expect(wrapper_source.find("const int success_int_value = copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host DLL wrapper should parse the typed success integer value from the success representation");
            expect(wrapper_source.find("const int fallback_int_value = copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host DLL wrapper should parse the typed fallback integer value from the fallback representation");
            expect(wrapper_source.find("const auto int_value_representation = std::to_string(int_value);") != std::string::npos,
                   "build host DLL wrapper should build typed return statements from parsed integer values");
            expect(wrapper_source.find("native_return_plan.success_int_value,") != std::string::npos,
                   "build host DLL wrapper should materialize success returns from the parsed success integer value");
            expect(wrapper_source.find("native_return_plan.fallback_int_value,") != std::string::npos,
                   "build host DLL wrapper should materialize fallback returns from the parsed fallback integer value");
            expect(wrapper_source.find("\"else { \" + return_materialization.fallback_return_statement + \" }\";") != std::string::npos,
                   "build host DLL wrapper should record an explicit fallback else-branch statement");
            expect(wrapper_source.find("success_branch_statement + \" \" + fallback_branch_statement;") != std::string::npos,
                   "build host DLL wrapper should compose the emitted return block from the explicit branch statements");
            expect(wrapper_source.find("const auto active_return_block = final_return_adoption.adopted_return_block;") != std::string::npos,
                   "build host DLL wrapper should seed the inactive active-return block from the adopted return block");
            expect(wrapper_source.find(": return_activation.active_return_block;") != std::string::npos,
                   "build host DLL wrapper should route the deferred stub-return block through the activation metadata");
            expect(wrapper_source.find("int placeholder_fallback_int_value = -1;") != std::string::npos,
                   "build host DLL wrapper should record placeholder fallback integers in the stub-return plan");
            expect(wrapper_source.find("std::string placeholder_fallback_value_representation;") != std::string::npos,
                   "build host DLL wrapper should record placeholder fallback representations in the stub-return plan");
            expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
                   "build host DLL wrapper should record placeholder-emission flags in the placeholder-return-value plan");
            expect(wrapper_source.find("std::string emitted_return_statement;") != std::string::npos,
                   "build host DLL wrapper should record placeholder emitted-return statements in the placeholder-return-value plan");
            expect(wrapper_source.find("std::string deferred_return_block;") != std::string::npos,
                   "build host DLL wrapper should record deferred return blocks in the placeholder-return-value plan");
            expect(wrapper_source.find("stub_return.emits_placeholder_return,") != std::string::npos,
                   "build host DLL wrapper should feed placeholder-emission flags from stub-return metadata");
            expect(wrapper_source.find("stub_return.emitted_return_statement,") != std::string::npos,
                   "build host DLL wrapper should feed emitted placeholder-return statements from stub-return metadata");
            expect(wrapper_source.find("stub_return.deferred_return_block,") != std::string::npos,
                   "build host DLL wrapper should feed deferred return blocks from stub-return metadata");
            expect(wrapper_source.find("stub_return.activation_mode,") != std::string::npos,
                   "build host DLL wrapper should feed activation modes from stub-return metadata");
            expect(wrapper_source.find("stub_return.adoption_mode,") != std::string::npos,
                   "build host DLL wrapper should feed adoption modes from stub-return metadata");
            expect(wrapper_source.find("stub_return.keeps_placeholder_return_active,") != std::string::npos,
                   "build host DLL wrapper should feed placeholder-helper active-policy booleans from stub-return metadata");
            expect(wrapper_source.find("stub_return.adopts_placeholder_replacement,") != std::string::npos,
                   "build host DLL wrapper should feed placeholder-helper replacement-policy booleans from stub-return metadata");
            expect(wrapper_source.find("stub_return.placeholder_fallback_int_value,") != std::string::npos,
                   "build host DLL wrapper should feed placeholder fallback integers from stub-return metadata");
            expect(wrapper_source.find("stub_return.placeholder_fallback_value_representation};") != std::string::npos,
                   "build host DLL wrapper should feed placeholder fallback representations from stub-return metadata");
            expect(wrapper_source.find("const bool keeps_placeholder_return_active =") != std::string::npos,
                   "build host DLL wrapper should derive placeholder-helper active-policy booleans upstream");
            expect(wrapper_source.find("const bool adopts_placeholder_replacement =") != std::string::npos,
                   "build host DLL wrapper should derive placeholder-helper replacement-policy booleans upstream");
            expect(wrapper_source.find("(void)placeholder_return_value.emitted_return_statement;") != std::string::npos,
                   "build host DLL wrapper should have the helper consume the placeholder emitted-return statement contract");
            expect(wrapper_source.find("(void)placeholder_return_value.deferred_return_block;") != std::string::npos,
                   "build host DLL wrapper should have the helper consume the deferred return-block contract");
            expect(wrapper_source.find("placeholder_return_value.keeps_placeholder_return_active") != std::string::npos,
                   "build host DLL wrapper should have the helper consume the routed active-policy boolean");
            expect(wrapper_source.find("placeholder_return_value.adopts_placeholder_replacement") != std::string::npos,
                   "build host DLL wrapper should have the helper consume the routed replacement-policy boolean");
            expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
                   "build host DLL wrapper should build an outcome selection plan from the native return plan");
            expect(wrapper_source.find("const auto outcome_selection_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged outcome selection from the native-return admission and outcome-selection plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);") != std::string::npos,
                   "build host DLL wrapper should route native-return admission through the shared outcome-selection admission helper.");
            expect(wrapper_source.find("(void)outcome_selection_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only outcome-selection admission result unused.");
            expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
                   "build host DLL wrapper should build a return materialization plan from the outcome selection plan");
            expect(wrapper_source.find("const auto return_materialization_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged return materialization from the outcome-selection admission and return-materialization plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);") != std::string::npos,
                   "build host DLL wrapper should route outcome-selection admission through the shared return-materialization admission helper.");
            expect(wrapper_source.find("(void)return_materialization_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only return-materialization admission result unused.");
            expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
                   "build host DLL wrapper should build a return emission plan from the return materialization plan");
            expect(wrapper_source.find("const auto return_emission_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged return emission from the return-materialization admission and return-emission plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);") != std::string::npos,
                   "build host DLL wrapper should route return-materialization admission through the shared return-emission admission helper.");
            expect(wrapper_source.find("(void)return_emission_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only return-emission admission result unused.");
            expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
                   "build host DLL wrapper should build a final return adoption plan from the return emission plan");
            expect(wrapper_source.find("const auto final_return_adoption_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged final return adoption from the return-emission admission and final-return-adoption plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);") != std::string::npos,
                   "build host DLL wrapper should route return-emission admission through the shared final-return-adoption admission helper.");
            expect(wrapper_source.find("(void)final_return_adoption_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only final-return-adoption admission result unused.");
            expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
                   "build host DLL wrapper should build a return activation plan from the final return adoption plan");
            expect(wrapper_source.find("const auto return_activation_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged return activation from the final-return-adoption admission and return-activation plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);") != std::string::npos,
                   "build host DLL wrapper should route final-return-adoption admission through the shared return-activation admission helper.");
            expect(wrapper_source.find("(void)return_activation_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only return-activation admission result unused.");
            expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
                   "build host DLL wrapper should build a stub return plan from the return activation plan");
            expect(wrapper_source.find("const auto stub_return_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged stub-return routing from the return-activation admission and stub-return plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);") != std::string::npos,
                   "build host DLL wrapper should route return-activation admission through the shared stub-return admission helper.");
            expect(wrapper_source.find("(void)stub_return_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only stub-return admission result unused.");
            expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
                   "build host DLL wrapper should build a placeholder-return-value plan from the stub return plan");
            expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n        stub_return_plan);") != std::string::npos,
                   "build host DLL wrapper should build the placeholder-return-value plan directly from the stub return plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto placeholder_return_value_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged placeholder-return-value routing from the stub-return admission and placeholder-return-value plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);") != std::string::npos,
                   "build host DLL wrapper should route stub-return admission through the shared placeholder-return-value admission helper.");
            expect(wrapper_source.find("(void)placeholder_return_value_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only placeholder-return-value admission result unused.");
            expect(wrapper_source.find("const auto placeholder_return_int_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged placeholder-return-int routing from the placeholder-return-value admission and placeholder-return-value plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);") != std::string::npos,
                   "build host DLL wrapper should route placeholder-return-value admission through the shared placeholder-return-int admission helper.");
            expect(wrapper_source.find("(void)placeholder_return_int_admission;") != std::string::npos,
                   "build host DLL wrapper should explicitly keep the scaffold-only placeholder-return-int admission result unused.");
            expect(wrapper_source.find("native_return_plan.fallback_int_value") != std::string::npos,
                   "build host DLL wrapper should propagate the typed native fallback integer value downstream");
            expect(wrapper_source.find("return copperfin_runtime_bridge_emit_stub_return_shared(placeholder_return_value_plan);") != std::string::npos,
                   "build host DLL wrapper should route the placeholder return through the plan-backed shared stub-emission emitter helper");
            expect(wrapper_source.find("\"--library-export\"") != std::string::npos,
                   "build host DLL wrapper should encode the export name into the bridge invocation plan");
            expect(wrapper_source.find("{\"tcMode\", std::to_string(tcMode), \"int\"}") != std::string::npos,
                   "build host DLL wrapper should preserve the DLL placeholder argument binding");
            expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
                   "build host DLL wrapper should feed the bridge result from the enriched descriptor and shared placeholder return binding");
            expect(wrapper_source.find("{copperfin_runtime_bridge_library_export_env_var(), result.call.invocation.descriptor.export_name}") != std::string::npos,
                   "build host DLL wrapper should preserve launch environment export metadata");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should derive stdout observation paths");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should derive stderr observation paths");
            expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.descriptor.runtime_host_path") != std::string::npos,
                   "build host DLL wrapper should preserve the runtime-host executable path in the execution plan");
            expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.arguments") != std::string::npos,
                   "build host DLL wrapper should preserve the bridge invocation arguments in the execution plan");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should derive request transport paths");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should derive response transport paths");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request serialization media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response serialization media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared serialization schema-version helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request-path dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-path dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request-media-type dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-media-type dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared schema-version dispatch helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
                   "build host DLL wrapper should route the request serialization media type through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
                   "build host DLL wrapper should route the response serialization media type through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
                   "build host DLL wrapper should route the serialization schema version through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the request-path dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the response-path dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the request-media-type dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the response-media-type dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the schema-version dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
                   "build host DLL wrapper should route the request payload shape through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
                   "build host DLL wrapper should route the response payload shape through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the export-name field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the parameter-count field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the parameters field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the request-media-type field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the response value field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the response-media-type field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the response status field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the response diagnostics field through the shared helper");
            expect(wrapper_source.find("        copperfin_build_runtime_bridge_native_int_return_surface());") != std::string::npos,
                   "build host DLL wrapper should preserve the DLL wrapper return surface");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
                   "build host DLL wrapper should declare the diagnostics fallback policy through the shared token helper");
            expect(wrapper_source.find("placeholder_return_binding.value_representation);") != std::string::npos,
                   "build host DLL wrapper should declare the fallback return value policy through the shared binding");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding)") != std::string::npos,
                   "build host DLL wrapper should derive the placeholder return statement from the shared binding helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
                   "build host DLL wrapper should declare the success-status expectation through the shared token helper");
            expect(wrapper_source.find("std::string request_document;") != std::string::npos,
                   "build host DLL wrapper should record the request document payload.");
            expect(wrapper_source.find("std::filesystem::path target_path;") != std::string::npos,
                   "build host DLL wrapper should record the request write target path.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request write-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
                   "build host DLL wrapper should route the request write mode through the shared helper.");
            expect(wrapper_source.find("std::filesystem::path source_path;") != std::string::npos,
                   "build host DLL wrapper should record the response read source path.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response read-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
                   "build host DLL wrapper should route the response read mode through the shared helper.");
            expect(wrapper_source.find("std::string response_document;") != std::string::npos,
                   "build host DLL wrapper should record the response document payload.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
                   "build host DLL wrapper should declare a shared empty response-document helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
                   "build host DLL wrapper should route the empty response-document token through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response parse-kind helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
                   "build host DLL wrapper should route the response parse kind through the shared helper.");
            expect(wrapper_source.find("std::string wrapper_return_surface;") != std::string::npos,
                   "build host DLL wrapper should record the wrapper return surface.");
            expect(wrapper_source.find("std::string native_return_surface;") != std::string::npos,
                   "build host DLL wrapper should record the native return surface.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
                   "build host DLL wrapper should declare a shared success-comparator helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
                   "build host DLL wrapper should declare a shared fallback-comparator helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
                   "build host DLL wrapper should route the success comparator through the shared helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
                   "build host DLL wrapper should route the fallback comparator through the shared helper.");
            expect(wrapper_source.find("std::string success_condition;") != std::string::npos,
                   "build host DLL wrapper should record the outcome success condition.");
            expect(wrapper_source.find("std::string success_return_statement;") != std::string::npos,
                   "build host DLL wrapper should record the success return statement.");
            expect(wrapper_source.find("std::string emitted_return_block;") != std::string::npos,
                   "build host DLL wrapper should record the emitted return block.");
            expect(wrapper_source.find("std::string placeholder_return_statement;") != std::string::npos,
                   "build host DLL wrapper should record the placeholder return statement.");
            expect(wrapper_source.find("bool activates_adopted_return = false;") != std::string::npos,
                   "build host DLL wrapper should record the inactive return-activation flag.");
            expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
                   "build host DLL wrapper should record the placeholder-emission flag.");
            expect(wrapper_source.find("int fallback_int_value = -1;") != std::string::npos,
                   "build host DLL wrapper should record the placeholder fallback integer value.");
            expect(wrapper_source.find("int success_int_value = -1;") != std::string::npos,
                   "build host DLL wrapper should record the typed native success integer value.");
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
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
                   "build host FLL wrapper should declare manifest flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
                   "build host FLL wrapper should route manifest flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
                   "build host FLL wrapper should declare library-export flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
                   "build host FLL wrapper should route library-export flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
                   "build host FLL wrapper should declare routine-kind flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
                   "build host FLL wrapper should route routine-kind flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
                   "build host FLL wrapper should declare source-path flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
                   "build host FLL wrapper should route source-path flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
                   "build host FLL wrapper should declare source-line flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
                   "build host FLL wrapper should route source-line flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
                   "build host FLL wrapper should declare parameter-declaration flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
                   "build host FLL wrapper should route parameter-declaration flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
                   "build host FLL wrapper should declare parameter-names flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
                   "build host FLL wrapper should route parameter-names flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
                   "build host FLL wrapper should declare parameter-count flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
                   "build host FLL wrapper should route parameter-count flag through helper");
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
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturn copperfin_build_runtime_bridge_placeholder_return_binding(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder return-binding helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeEnvironmentVariable") != std::string::npos,
                   "build host FLL wrapper should declare a launch-environment surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeLaunchPlan") != std::string::npos,
                   "build host FLL wrapper should declare a launch-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeLaunchPlan copperfin_build_runtime_bridge_launch_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a launch-plan helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
                   "build host FLL wrapper should declare library-export env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
                   "build host FLL wrapper should route library-export env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
                   "build host FLL wrapper should declare routine-kind env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
                   "build host FLL wrapper should route routine-kind env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
                   "build host FLL wrapper should declare source-path env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
                   "build host FLL wrapper should route source-path env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
                   "build host FLL wrapper should declare parameter-count env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
                   "build host FLL wrapper should route parameter-count env-var through helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeObservationPlan") != std::string::npos,
                   "build host FLL wrapper should declare an observation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeObservationPlan copperfin_build_runtime_bridge_observation_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an observation-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeExecutionPlan") != std::string::npos,
                   "build host FLL wrapper should declare an execution-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeExecutionPlan copperfin_build_runtime_bridge_execution_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an execution-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeTransportPlan") != std::string::npos,
                   "build host FLL wrapper should declare a transport-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeTransportPlan copperfin_build_runtime_bridge_transport_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a transport-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeSerializationPlan") != std::string::npos,
                   "build host FLL wrapper should declare a serialization-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeSerializationPlan copperfin_build_runtime_bridge_serialization_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a serialization-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeDispatchPlan") != std::string::npos,
                   "build host FLL wrapper should declare a dispatch-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchPlan copperfin_build_runtime_bridge_dispatch_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a dispatch-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchExecution copperfin_runtime_bridge_execute_dispatch(") != std::string::npos,
                   "build host FLL wrapper should declare a shared dispatch-execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_launch_process(") != std::string::npos,
                   "build host FLL wrapper should declare a shared process-launch helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeHostFailureEvaluation copperfin_runtime_bridge_evaluate_host_failure(") != std::string::npos,
                   "build host FLL wrapper should declare a shared host-failure evaluation helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeMissingResponseEvaluation copperfin_runtime_bridge_evaluate_missing_response(") != std::string::npos,
                   "build host FLL wrapper should declare a shared missing-response evaluation helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationEvaluation copperfin_runtime_bridge_evaluate_response_validation(") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-validation evaluation helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgePayloadPlan") != std::string::npos,
                   "build host FLL wrapper should declare a payload-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePayloadPlan copperfin_build_runtime_bridge_payload_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a payload-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretationPlan") != std::string::npos,
                   "build host FLL wrapper should declare an interpretation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretationPlan copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an interpretation-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeFailurePolicyPlan") != std::string::npos,
                   "build host FLL wrapper should declare a failure-policy surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFailurePolicyPlan copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a failure-policy helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-status field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-value field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-diagnostics field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request payload-shape helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response payload-shape helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared export-name field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-count field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameters field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request-media-type field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-media-type field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared payload-shape field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
                   "build host FLL wrapper should route payload-shape field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-name field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
                   "build host FLL wrapper should route parameter-name field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-value field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
                   "build host FLL wrapper should route parameter-value field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-surface field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
                   "build host FLL wrapper should route parameter-surface field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared failure-diagnostics token helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared success-status token helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseValidationPlan") != std::string::npos,
                   "build host FLL wrapper should declare a response-validation surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationPlan copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a response-validation helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestArtifact") != std::string::npos,
                   "build host FLL wrapper should declare a request-artifact surface");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_document(") != std::string::npos,
                   "build host FLL wrapper should declare a request-document helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestArtifact copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
                   "build host FLL wrapper should declare a request-artifact helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestWritePlan") != std::string::npos,
                   "build host FLL wrapper should declare a request-write-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestWritePlan copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a request-write-plan helper");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_execute_write_request(") != std::string::npos,
                   "build host FLL wrapper should declare a shared request-write execution helper.");
            expect(wrapper_source.find("out << plan.request_artifact.request_document;") != std::string::npos,
                   "build host FLL wrapper should stage request-document writes through the shared request-write execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseReadPlan") != std::string::npos,
                   "build host FLL wrapper should declare a response-read-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseReadPlan copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a response-read-plan helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_execute_read_response(") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-read execution helper.");
            expect(wrapper_source.find("response_document << input.rdbuf();") != std::string::npos,
                   "build host FLL wrapper should stage response-document reads through the shared response-read execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseArtifact") != std::string::npos,
                   "build host FLL wrapper should declare a response-artifact surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseArtifact copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
                   "build host FLL wrapper should declare a response-artifact helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseParsePlan") != std::string::npos,
                   "build host FLL wrapper should declare a response-parse-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParsePlan copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a response-parse-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParseAdmission copperfin_runtime_bridge_admit_response_parse(") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-parse admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeParsedResponse copperfin_runtime_bridge_execute_parse_response(") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-parse execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(response_document, plan.status_field)") != std::string::npos,
                   "build host FLL wrapper should stage response field extraction through the shared response-parse execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretedResultPlan") != std::string::npos,
                   "build host FLL wrapper should declare an interpreted-result-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultPlan copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an interpreted-result-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultAdmission copperfin_runtime_bridge_admit_interpreted_result(") != std::string::npos,
                   "build host FLL wrapper should declare a shared interpreted-result admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResult copperfin_runtime_bridge_execute_interpreted_result(") != std::string::npos,
                   "build host FLL wrapper should declare a shared interpreted-result execution helper.");
            expect(wrapper_source.find("parsed_response.status_value == plan.success_status_value") != std::string::npos,
                   "build host FLL wrapper should stage interpreted-result selection through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeNativeReturnPlan") != std::string::npos,
                   "build host FLL wrapper should declare a native-return-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnPlan copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a native-return-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnAdmission copperfin_runtime_bridge_admit_native_return(") != std::string::npos,
                   "build host FLL wrapper should declare a shared native-return admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturn copperfin_runtime_bridge_execute_native_return(") != std::string::npos,
                   "build host FLL wrapper should declare a shared native-return execution helper.");
            expect(wrapper_source.find("interpreted_result.matched_success_status") != std::string::npos,
                   "build host FLL wrapper should stage native-return selection through the shared execution helper.");
            expect(wrapper_source.find("static int copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host FLL wrapper should declare an integer return-representation parser");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_default_int_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parsed-int default sentinel helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_default_int_value()") != std::string::npos,
                   "build host FLL wrapper should route the parsed-int default sentinel through the shared helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeOutcomeSelectionPlan") != std::string::npos,
                   "build host FLL wrapper should declare an outcome-selection-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionPlan copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an outcome-selection-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionAdmission copperfin_runtime_bridge_admit_outcome_selection(") != std::string::npos,
                   "build host FLL wrapper should declare a shared outcome-selection admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelection copperfin_runtime_bridge_execute_outcome_selection(") != std::string::npos,
                   "build host FLL wrapper should declare a shared outcome-selection execution helper.");
            expect(wrapper_source.find("native_return.matched_success_status") != std::string::npos,
                   "build host FLL wrapper should stage outcome selection through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnMaterializationPlan") != std::string::npos,
                   "build host FLL wrapper should declare a return-materialization-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationPlan copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a return-materialization-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationAdmission copperfin_runtime_bridge_admit_return_materialization(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-materialization admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterialization copperfin_runtime_bridge_execute_return_materialization(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-materialization execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_outcome_selection(plan.outcome_selection_plan)") != std::string::npos,
                   "build host FLL wrapper should stage return materialization through the shared execution helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
                   "build host FLL wrapper should declare a shared native-int return-surface helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
                   "build host FLL wrapper should route native-int return-surface comparisons through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_placeholder_signature_token()") != std::string::npos,
                   "build host FLL wrapper should declare a shared native-int placeholder-signature helper.");
            expect(wrapper_source.find("find(copperfin_build_runtime_bridge_native_int_placeholder_signature_token())") != std::string::npos,
                   "build host FLL wrapper should route native-int placeholder-signature matching through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
                   "build host FLL wrapper should declare a shared native return-statement framing helper.");
            expect(wrapper_source.find("return copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
                   "build host FLL wrapper should route native return-statement framing through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
                   "build host FLL wrapper should declare a shared typed native return-expression helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
                   "build host FLL wrapper should route typed native return-expression construction through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should declare a shared stdout log-file suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should route stdout log-file suffix through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should declare a shared stderr log-file suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should route stderr log-file suffix through the shared helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
                   "build host FLL wrapper should declare a shared expected-exit-code helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
                   "build host FLL wrapper should route expected-exit-code through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request artifact suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should route request artifact suffix through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response artifact suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should route response artifact suffix through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared activates-adopted-return policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
                   "build host FLL wrapper should route activates-adopted-return policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared capture-stdout policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
                   "build host FLL wrapper should route capture-stdout policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared capture-stderr policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
                   "build host FLL wrapper should route capture-stderr policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared fail-on-nonzero-exit policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
                   "build host FLL wrapper should route fail-on-nonzero-exit policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared fail-on-missing-response policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
                   "build host FLL wrapper should route fail-on-missing-response policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared ensure-parent-directory policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
                   "build host FLL wrapper should route ensure-parent-directory policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared require-existing-response policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
                   "build host FLL wrapper should route require-existing-response policy through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
                   "build host FLL wrapper should declare a shared replace-placeholder-return adoption-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
                   "build host FLL wrapper should route replace-placeholder-return mode token through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
                   "build host FLL wrapper should declare a shared planned-activation-pending activation-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
                   "build host FLL wrapper should route planned-activation-pending mode token through the shared helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnEmissionPlan") != std::string::npos,
                   "build host FLL wrapper should declare a return-emission-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionPlan copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a return-emission-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionAdmission copperfin_runtime_bridge_admit_return_emission(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-emission admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmission copperfin_runtime_bridge_execute_return_emission(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-emission execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_return_materialization(plan.return_materialization_plan)") != std::string::npos,
                   "build host FLL wrapper should stage return emission through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeFinalReturnAdoptionPlan") != std::string::npos,
                   "build host FLL wrapper should declare a final-return-adoption-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionPlan copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a final-return-adoption-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionAdmission copperfin_runtime_bridge_admit_final_return_adoption(") != std::string::npos,
                   "build host FLL wrapper should declare a shared final-return-adoption admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoption copperfin_runtime_bridge_execute_final_return_adoption(") != std::string::npos,
                   "build host FLL wrapper should declare a shared final-return-adoption execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_return_emission(plan.return_emission_plan)") != std::string::npos,
                   "build host FLL wrapper should stage final-return adoption through the shared execution helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_placeholder_return_statement(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder return-statement helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnActivationPlan") != std::string::npos,
                   "build host FLL wrapper should declare a return-activation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationPlan copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a return-activation-plan helper");
            expect(wrapper_source.find("CopperfinRuntimeBridgeStubEmissionWrapper stub_emission_wrapper;") != std::string::npos,
                   "build host FLL wrapper should carry the stub-emission wrapper contract through the descriptor plan.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationAdmission copperfin_runtime_bridge_admit_return_activation(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-activation admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivation copperfin_runtime_bridge_execute_return_activation(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-activation execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_final_return_adoption(plan.final_return_adoption_plan)") != std::string::npos,
                   "build host FLL wrapper should stage return activation through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubReturnPlan") != std::string::npos,
                   "build host FLL wrapper should declare a stub-return-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnPlan copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a stub-return-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnAdmission copperfin_runtime_bridge_admit_stub_return(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-return admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturn copperfin_runtime_bridge_execute_stub_return(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-return execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_return_activation(plan.return_activation_plan)") != std::string::npos,
                   "build host FLL wrapper should stage stub-return handling through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgePlaceholderReturnValuePlan") != std::string::npos,
                   "build host FLL wrapper should declare a placeholder-return-value-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValuePlan copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a placeholder-return-value-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValueAdmission copperfin_runtime_bridge_admit_placeholder_return_value(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder-return-value admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValue copperfin_runtime_bridge_execute_placeholder_return_value(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder-return-value execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_stub_return(plan.stub_return_plan)") != std::string::npos,
                   "build host FLL wrapper should stage placeholder-return-value handling through the shared execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnIntAdmission copperfin_runtime_bridge_admit_placeholder_return_int(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder-return-int admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionAdmission copperfin_runtime_bridge_admit_stub_emission(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmission copperfin_runtime_bridge_execute_stub_emission(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionReturnSurface copperfin_runtime_bridge_build_stub_emission_return_surface(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission return-surface helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_apply_stub_emission_output(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission output-application helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubEmissionWrapper") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission wrapper surface.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionWrapper copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission wrapper helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return_shared(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission emitter helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_execute_placeholder_return_int(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder-return-int execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_value)};") != std::string::npos,
                   "build host FLL wrapper should route placeholder-return-int execution through the shared stub-emission helper.");
            expect(wrapper_source.find("const auto stub_emission_wrapper =\n        copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
                   "build host FLL wrapper should build a shared stub-emission wrapper before building the descriptor plan.");
            expect(wrapper_source.find("return copperfin_runtime_bridge_emit_stub_return_shared(") != std::string::npos,
                   "build host FLL wrapper should route FLL stub emission through the shared emitter helper.");
            expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.native_return_surface") != std::string::npos,
                   "build host FLL wrapper should read the stub-emission return surface through the descriptor plan.");
            expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter") != std::string::npos,
                   "build host FLL wrapper should read the stub-emission return adapter through the descriptor plan.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_fll_int_return_surface(),") != std::string::npos,
                   "build host FLL wrapper should pass the FLL int return-surface contract into the shared wrapper helper.");
            expect(wrapper_source.find("_RetInt);") != std::string::npos,
                   "build host FLL wrapper should pass the `_RetInt` adapter into the shared wrapper helper.");
            expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
                   "build host FLL wrapper should pass the built wrapper into the descriptor-plan builder.");
            expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n        interpretation_plan,\n        placeholder_return_binding.value_representation);") != std::string::npos,
                   "build host FLL wrapper should build the failure-policy plan from the enriched interpretation plan.");
            expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n        failure_policy);") != std::string::npos,
                   "build host FLL wrapper should build the response-validation plan from the enriched failure-policy plan.");
            expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n        response_validation);") != std::string::npos,
                   "build host FLL wrapper should build the request artifact directly from the response-validation plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n        request_artifact);") != std::string::npos,
                   "build host FLL wrapper should build the request-write plan directly from the request artifact once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n        request_write_plan);") != std::string::npos,
                   "build host FLL wrapper should build the response-read plan directly from the request-write plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto response_document =\n        copperfin_runtime_bridge_execute_read_response(response_read_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the response-read plan before building the response artifact.");
            expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n        response_read_plan,\n        response_document);") != std::string::npos,
                   "build host FLL wrapper should build the response artifact from the response-read plan and executed response document.");
            expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n        response_artifact);") != std::string::npos,
                   "build host FLL wrapper should build the response-parse plan directly from the response artifact once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto parsed_response =\n        copperfin_runtime_bridge_execute_parse_response(response_parse_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the response-parse plan before building the interpreted-result plan.");
            expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n        response_parse_plan,\n        parsed_response);") != std::string::npos,
                   "build host FLL wrapper should build the interpreted-result plan from the response-parse plan and parsed response.");
            expect(wrapper_source.find("const auto interpreted_result =\n        copperfin_runtime_bridge_execute_interpreted_result(interpreted_result_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the interpreted-result plan before building the native-return plan.");
            expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n        result,\n        interpreted_result_plan,\n        interpreted_result);") != std::string::npos,
                   "build host FLL wrapper should build the native-return plan from the interpreted-result plan and interpreted result.");
            expect(wrapper_source.find("const auto native_return =\n        copperfin_runtime_bridge_execute_native_return(native_return_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the native-return plan before building the outcome-selection plan.");
            expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n        native_return_plan,\n        native_return);") != std::string::npos,
                   "build host FLL wrapper should build the outcome-selection plan from the native-return plan and native return.");
            expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n        outcome_selection_plan);") != std::string::npos,
                   "build host FLL wrapper should build the return-materialization plan directly from the outcome-selection plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n        return_materialization_plan);") != std::string::npos,
                   "build host FLL wrapper should build the return-emission plan directly from the return-materialization plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n        return_emission_plan,\n        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));") != std::string::npos,
                   "build host FLL wrapper should build the final-return-adoption plan directly from the return-emission plan once the wrapper contract is carried by response validation.");
            expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n        final_return_adoption_plan);") != std::string::npos,
                   "build host FLL wrapper should build the return-activation plan directly from the final-return-adoption plan once the wrapper contract is carried by response validation.");
            expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n        return_activation_plan);") != std::string::npos,
                   "build host FLL wrapper should build the stub-return plan directly from the return-activation plan once the wrapper contract is carried by response validation.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return(\n") == std::string::npos,
                   "build host FLL wrapper should no longer declare an output-specific stub-return wrapper helper once the plan carries the wrapper contract.");
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
            expect(wrapper_source.find("const auto invocation = copperfin_build_runtime_bridge_invocation(\n        descriptor);") != std::string::npos,
                   "build host FLL wrapper should build a bridge invocation from the descriptor");
            expect(wrapper_source.find("const auto call = copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host FLL wrapper should build a bridge call from the invocation");
            expect(wrapper_source.find("const auto result = copperfin_build_runtime_bridge_result(\n        call,\n        placeholder_return_binding);") != std::string::npos,
                   "build host FLL wrapper should build a bridge result from the enriched call");
            expect(wrapper_source.find("const auto placeholder_return_binding =") != std::string::npos,
                   "build host FLL wrapper should build a shared placeholder return binding before building the result");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_fll_int_return_surface()") != std::string::npos,
                   "build host FLL wrapper should declare a shared FLL int return-surface helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_fll_int_return_surface()") != std::string::npos,
                   "build host FLL wrapper should route FLL return surface through helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_binding(\n            copperfin_build_runtime_bridge_fll_int_return_surface())") != std::string::npos,
                   "build host FLL wrapper should build the FLL placeholder return binding through the shared helper");
            expect(wrapper_source.find("const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(\n        result);") != std::string::npos,
                   "build host FLL wrapper should build a launch plan from the result");
            expect(wrapper_source.find("const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(\n        launch_plan);") != std::string::npos,
                   "build host FLL wrapper should build an observation plan from the launch plan");
            expect(wrapper_source.find("const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(\n        observation_plan);") != std::string::npos,
                   "build host FLL wrapper should build an execution plan from the observation plan");
            expect(wrapper_source.find("const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(\n        execution_plan);") != std::string::npos,
                   "build host FLL wrapper should build a transport plan from the execution plan");
            expect(wrapper_source.find("const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(\n        transport_plan);") != std::string::npos,
                   "build host FLL wrapper should build a serialization plan from the transport plan");
            expect(wrapper_source.find("const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(\n        serialization_plan);") != std::string::npos,
                   "build host FLL wrapper should build a dispatch plan from the serialization plan");
            expect(wrapper_source.find("const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);") != std::string::npos,
                   "build host FLL wrapper should route the dispatch plan through the shared dispatch-execution helper.");
            expect(wrapper_source.find("(void)dispatch_execution;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only dispatch-execution result unused.");
            expect(wrapper_source.find("const auto process_launch = copperfin_runtime_bridge_launch_process(dispatch_execution);") != std::string::npos,
                   "build host FLL wrapper should route dispatch execution through the shared process-launch helper.");
            expect(wrapper_source.find("(void)process_launch;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only process-launch result unused.");
            expect(wrapper_source.find("const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(\n        dispatch_plan);") != std::string::npos,
                   "build host FLL wrapper should build a payload plan from the dispatch plan");
            expect(wrapper_source.find("const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
                   "build host FLL wrapper should build an interpretation plan from the payload plan");
            expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
                   "build host FLL wrapper should build a failure policy from the interpretation plan");
            expect(wrapper_source.find("const auto host_failure =") != std::string::npos,
                   "build host FLL wrapper should evaluate staged host failure from the process-launch helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);") != std::string::npos,
                   "build host FLL wrapper should route process-launch output through the shared host-failure evaluation helper.");
            expect(wrapper_source.find("(void)host_failure;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only host-failure evaluation result unused.");
            expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
                   "build host FLL wrapper should build a response-validation plan from the failure policy");
            expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
                   "build host FLL wrapper should build a request artifact from the response validation plan");
            expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
                   "build host FLL wrapper should build a request write plan from the request artifact");
            expect(wrapper_source.find("const auto request_write_execution =\n        copperfin_runtime_bridge_execute_write_request(request_write_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the request-write plan through the shared helper.");
            expect(wrapper_source.find("(void)request_write_execution;") != std::string::npos,
                   "build host FLL wrapper should explicitly discard the scaffold-only request-write execution result.");
            expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
                   "build host FLL wrapper should build a response read plan from the request write plan");
            expect(wrapper_source.find("const auto missing_response =") != std::string::npos,
                   "build host FLL wrapper should evaluate staged missing-response policy from the host-failure and response-read helpers.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_missing_response(host_failure, response_read_plan);") != std::string::npos,
                   "build host FLL wrapper should route host-failure output through the shared missing-response evaluation helper.");
            expect(wrapper_source.find("(void)missing_response;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only missing-response evaluation result unused.");
            expect(wrapper_source.find("const auto response_validation_evaluation =") != std::string::npos,
                   "build host FLL wrapper should evaluate staged response-validation policy from the missing-response and validation helpers.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_response_validation(missing_response, response_validation);") != std::string::npos,
                   "build host FLL wrapper should route missing-response output through the shared response-validation evaluation helper.");
            expect(wrapper_source.find("(void)response_validation_evaluation;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only response-validation evaluation result unused.");
            expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
                   "build host FLL wrapper should build a response artifact from the response read plan");
            expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
                   "build host FLL wrapper should build a response parse plan from the response artifact");
            expect(wrapper_source.find("const auto response_parse_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged response parsing from the response-validation evaluation and parse plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);") != std::string::npos,
                   "build host FLL wrapper should route response-validation evaluation through the shared response-parse admission helper.");
            expect(wrapper_source.find("(void)response_parse_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only response-parse admission result unused.");
            expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
                   "build host FLL wrapper should build an interpreted result plan from the response parse plan");
            expect(wrapper_source.find("const auto interpreted_result_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged interpreted-result selection from the response-parse admission and interpreted-result plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);") != std::string::npos,
                   "build host FLL wrapper should route response-parse admission through the shared interpreted-result admission helper.");
            expect(wrapper_source.find("(void)interpreted_result_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only interpreted-result admission result unused.");
            expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
                   "build host FLL wrapper should build a native return plan from the interpreted result plan");
            expect(wrapper_source.find("const auto native_return_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged native-return selection from the interpreted-result admission and native-return plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);") != std::string::npos,
                   "build host FLL wrapper should route interpreted-result admission through the shared native-return admission helper.");
            expect(wrapper_source.find("(void)native_return_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only native-return admission result unused.");
            expect(wrapper_source.find("const int success_int_value = copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host FLL wrapper should parse the typed success integer value from the success representation");
            expect(wrapper_source.find("const int fallback_int_value = copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host FLL wrapper should parse the typed fallback integer value from the fallback representation");
            expect(wrapper_source.find("const auto int_value_representation = std::to_string(int_value);") != std::string::npos,
                   "build host FLL wrapper should build typed return statements from parsed integer values");
            expect(wrapper_source.find("native_return_plan.success_int_value,") != std::string::npos,
                   "build host FLL wrapper should materialize success returns from the parsed success integer value");
            expect(wrapper_source.find("native_return_plan.fallback_int_value,") != std::string::npos,
                   "build host FLL wrapper should materialize fallback returns from the parsed fallback integer value");
            expect(wrapper_source.find("\"else { \" + return_materialization.fallback_return_statement + \" }\";") != std::string::npos,
                   "build host FLL wrapper should record an explicit fallback else-branch statement");
            expect(wrapper_source.find("success_branch_statement + \" \" + fallback_branch_statement;") != std::string::npos,
                   "build host FLL wrapper should compose the emitted return block from the explicit branch statements");
            expect(wrapper_source.find("const auto active_return_block = final_return_adoption.adopted_return_block;") != std::string::npos,
                   "build host FLL wrapper should seed the inactive active-return block from the adopted return block");
            expect(wrapper_source.find(": return_activation.active_return_block;") != std::string::npos,
                   "build host FLL wrapper should route the deferred stub-return block through the activation metadata");
            expect(wrapper_source.find("int placeholder_fallback_int_value = -1;") != std::string::npos,
                   "build host FLL wrapper should record placeholder fallback integers in the stub-return plan");
            expect(wrapper_source.find("std::string placeholder_fallback_value_representation;") != std::string::npos,
                   "build host FLL wrapper should record placeholder fallback representations in the stub-return plan");
            expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
                   "build host FLL wrapper should record placeholder-emission flags in the placeholder-return-value plan");
            expect(wrapper_source.find("std::string emitted_return_statement;") != std::string::npos,
                   "build host FLL wrapper should record placeholder emitted-return statements in the placeholder-return-value plan");
            expect(wrapper_source.find("std::string deferred_return_block;") != std::string::npos,
                   "build host FLL wrapper should record deferred return blocks in the placeholder-return-value plan");
            expect(wrapper_source.find("stub_return.emits_placeholder_return,") != std::string::npos,
                   "build host FLL wrapper should feed placeholder-emission flags from stub-return metadata");
            expect(wrapper_source.find("stub_return.emitted_return_statement,") != std::string::npos,
                   "build host FLL wrapper should feed emitted placeholder-return statements from stub-return metadata");
            expect(wrapper_source.find("stub_return.deferred_return_block,") != std::string::npos,
                   "build host FLL wrapper should feed deferred return blocks from stub-return metadata");
            expect(wrapper_source.find("stub_return.activation_mode,") != std::string::npos,
                   "build host FLL wrapper should feed activation modes from stub-return metadata");
            expect(wrapper_source.find("stub_return.adoption_mode,") != std::string::npos,
                   "build host FLL wrapper should feed adoption modes from stub-return metadata");
            expect(wrapper_source.find("stub_return.keeps_placeholder_return_active,") != std::string::npos,
                   "build host FLL wrapper should feed placeholder-helper active-policy booleans from stub-return metadata");
            expect(wrapper_source.find("stub_return.adopts_placeholder_replacement,") != std::string::npos,
                   "build host FLL wrapper should feed placeholder-helper replacement-policy booleans from stub-return metadata");
            expect(wrapper_source.find("stub_return.placeholder_fallback_int_value,") != std::string::npos,
                   "build host FLL wrapper should feed placeholder fallback integers from stub-return metadata");
            expect(wrapper_source.find("stub_return.placeholder_fallback_value_representation};") != std::string::npos,
                   "build host FLL wrapper should feed placeholder fallback representations from stub-return metadata");
            expect(wrapper_source.find("const bool keeps_placeholder_return_active =") != std::string::npos,
                   "build host FLL wrapper should derive placeholder-helper active-policy booleans upstream");
            expect(wrapper_source.find("const bool adopts_placeholder_replacement =") != std::string::npos,
                   "build host FLL wrapper should derive placeholder-helper replacement-policy booleans upstream");
            expect(wrapper_source.find("(void)placeholder_return_value.emitted_return_statement;") != std::string::npos,
                   "build host FLL wrapper should have the helper consume the placeholder emitted-return statement contract");
            expect(wrapper_source.find("(void)placeholder_return_value.deferred_return_block;") != std::string::npos,
                   "build host FLL wrapper should have the helper consume the deferred return-block contract");
            expect(wrapper_source.find("placeholder_return_value.keeps_placeholder_return_active") != std::string::npos,
                   "build host FLL wrapper should have the helper consume the routed active-policy boolean");
            expect(wrapper_source.find("placeholder_return_value.adopts_placeholder_replacement") != std::string::npos,
                   "build host FLL wrapper should have the helper consume the routed replacement-policy boolean");
            expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
                   "build host FLL wrapper should build an outcome selection plan from the native return plan");
            expect(wrapper_source.find("const auto outcome_selection_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged outcome selection from the native-return admission and outcome-selection plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);") != std::string::npos,
                   "build host FLL wrapper should route native-return admission through the shared outcome-selection admission helper.");
            expect(wrapper_source.find("(void)outcome_selection_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only outcome-selection admission result unused.");
            expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
                   "build host FLL wrapper should build a return materialization plan from the outcome selection plan");
            expect(wrapper_source.find("const auto return_materialization_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged return materialization from the outcome-selection admission and return-materialization plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);") != std::string::npos,
                   "build host FLL wrapper should route outcome-selection admission through the shared return-materialization admission helper.");
            expect(wrapper_source.find("(void)return_materialization_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only return-materialization admission result unused.");
            expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
                   "build host FLL wrapper should build a return emission plan from the return materialization plan");
            expect(wrapper_source.find("const auto return_emission_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged return emission from the return-materialization admission and return-emission plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);") != std::string::npos,
                   "build host FLL wrapper should route return-materialization admission through the shared return-emission admission helper.");
            expect(wrapper_source.find("(void)return_emission_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only return-emission admission result unused.");
            expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
                   "build host FLL wrapper should build a final return adoption plan from the return emission plan");
            expect(wrapper_source.find("const auto final_return_adoption_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged final return adoption from the return-emission admission and final-return-adoption plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);") != std::string::npos,
                   "build host FLL wrapper should route return-emission admission through the shared final-return-adoption admission helper.");
            expect(wrapper_source.find("(void)final_return_adoption_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only final-return-adoption admission result unused.");
            expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
                   "build host FLL wrapper should build a return activation plan from the final return adoption plan");
            expect(wrapper_source.find("const auto return_activation_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged return activation from the final-return-adoption admission and return-activation plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);") != std::string::npos,
                   "build host FLL wrapper should route final-return-adoption admission through the shared return-activation admission helper.");
            expect(wrapper_source.find("(void)return_activation_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only return-activation admission result unused.");
            expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
                   "build host FLL wrapper should build a stub return plan from the return activation plan");
            expect(wrapper_source.find("const auto stub_return_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged stub-return routing from the return-activation admission and stub-return plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);") != std::string::npos,
                   "build host FLL wrapper should route return-activation admission through the shared stub-return admission helper.");
            expect(wrapper_source.find("(void)stub_return_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only stub-return admission result unused.");
            expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
                   "build host FLL wrapper should build a placeholder-return-value plan from the stub return plan");
            expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n        stub_return_plan);") != std::string::npos,
                   "build host FLL wrapper should build the placeholder-return-value plan directly from the stub return plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto placeholder_return_value_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged placeholder-return-value routing from the stub-return admission and placeholder-return-value plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);") != std::string::npos,
                   "build host FLL wrapper should route stub-return admission through the shared placeholder-return-value admission helper.");
            expect(wrapper_source.find("(void)placeholder_return_value_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only placeholder-return-value admission result unused.");
            expect(wrapper_source.find("const auto placeholder_return_int_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged placeholder-return-int routing from the placeholder-return-value admission and placeholder-return-value plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);") != std::string::npos,
                   "build host FLL wrapper should route placeholder-return-value admission through the shared placeholder-return-int admission helper.");
            expect(wrapper_source.find("(void)placeholder_return_int_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only placeholder-return-int admission result unused.");
            expect(wrapper_source.find("const auto stub_emission_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged stub emission from the placeholder-return-int admission.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission);") != std::string::npos,
                   "build host FLL wrapper should route placeholder-return-int admission through the shared stub-emission admission helper.");
            expect(wrapper_source.find("(void)stub_emission_admission;") != std::string::npos,
                   "build host FLL wrapper should explicitly keep the scaffold-only stub-emission admission result unused.");
            expect(wrapper_source.find("native_return_plan.fallback_int_value") != std::string::npos,
                   "build host FLL wrapper should propagate the typed native fallback integer value downstream");
            expect(wrapper_source.find("return copperfin_runtime_bridge_emit_stub_return_shared(placeholder_return_value_plan);") != std::string::npos,
                   "build host FLL wrapper should route the placeholder return through the plan-backed shared stub-emission emitter helper");
            expect(wrapper_source.find("\"--library-export\"") != std::string::npos,
                   "build host FLL wrapper should encode the export name into the bridge invocation plan");
            expect(wrapper_source.find("{{\"parm\", std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(parm))), \"ParamBlk*\"}}") != std::string::npos,
                   "build host FLL wrapper should preserve the ParamBlk call-surface binding");
            expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
                   "build host FLL wrapper should feed the bridge result from the enriched descriptor and shared placeholder return binding");
            expect(wrapper_source.find("{copperfin_runtime_bridge_library_export_env_var(), result.call.invocation.descriptor.export_name}") != std::string::npos,
                   "build host FLL wrapper should preserve launch environment export metadata");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should derive stdout observation paths");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should derive stderr observation paths");
            expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.descriptor.runtime_host_path") != std::string::npos,
                   "build host FLL wrapper should preserve the runtime-host executable path in the execution plan");
            expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.arguments") != std::string::npos,
                   "build host FLL wrapper should preserve the bridge invocation arguments in the execution plan");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should derive request transport paths");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should derive response transport paths");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request serialization media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response serialization media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared serialization schema-version helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request-path dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-path dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request-media-type dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-media-type dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared schema-version dispatch helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
                   "build host FLL wrapper should route the request serialization media type through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
                   "build host FLL wrapper should route the response serialization media type through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
                   "build host FLL wrapper should route the serialization schema version through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the request-path dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the response-path dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the request-media-type dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the response-media-type dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the schema-version dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
                   "build host FLL wrapper should route the request payload shape through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
                   "build host FLL wrapper should route the response payload shape through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the export-name field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the parameter-count field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the parameters field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the request-media-type field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the response value field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the response-media-type field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the response status field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the response diagnostics field through the shared helper");
            expect(wrapper_source.find("        copperfin_build_runtime_bridge_fll_int_return_surface());") != std::string::npos,
                   "build host FLL wrapper should preserve the FLL wrapper return surface");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
                   "build host FLL wrapper should declare the diagnostics fallback policy through the shared token helper");
            expect(wrapper_source.find("placeholder_return_binding.value_representation);") != std::string::npos,
                   "build host FLL wrapper should declare the fallback return value policy through the shared binding");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding)") != std::string::npos,
                   "build host FLL wrapper should derive the placeholder return statement from the shared binding helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
                   "build host FLL wrapper should declare the success-status expectation through the shared token helper");
            expect(wrapper_source.find("std::string request_document;") != std::string::npos,
                   "build host FLL wrapper should record the request document payload.");
            expect(wrapper_source.find("std::filesystem::path target_path;") != std::string::npos,
                   "build host FLL wrapper should record the request write target path.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request write-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
                   "build host FLL wrapper should route the request write mode through the shared helper.");
            expect(wrapper_source.find("std::filesystem::path source_path;") != std::string::npos,
                   "build host FLL wrapper should record the response read source path.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response read-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
                   "build host FLL wrapper should route the response read mode through the shared helper.");
            expect(wrapper_source.find("std::string response_document;") != std::string::npos,
                   "build host FLL wrapper should record the response document payload.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
                   "build host FLL wrapper should declare a shared empty response-document helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
                   "build host FLL wrapper should route the empty response-document token through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response parse-kind helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
                   "build host FLL wrapper should route the response parse kind through the shared helper.");
            expect(wrapper_source.find("std::string wrapper_return_surface;") != std::string::npos,
                   "build host FLL wrapper should record the wrapper return surface.");
            expect(wrapper_source.find("std::string native_return_surface;") != std::string::npos,
                   "build host FLL wrapper should record the native return surface.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
                   "build host FLL wrapper should declare a shared success-comparator helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
                   "build host FLL wrapper should declare a shared fallback-comparator helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
                   "build host FLL wrapper should route the success comparator through the shared helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
                   "build host FLL wrapper should route the fallback comparator through the shared helper.");
            expect(wrapper_source.find("std::string success_condition;") != std::string::npos,
                   "build host FLL wrapper should record the outcome success condition.");
            expect(wrapper_source.find("std::string success_return_statement;") != std::string::npos,
                   "build host FLL wrapper should record the success return statement.");
            expect(wrapper_source.find("std::string emitted_return_block;") != std::string::npos,
                   "build host FLL wrapper should record the emitted return block.");
            expect(wrapper_source.find("std::string placeholder_return_statement;") != std::string::npos,
                   "build host FLL wrapper should record the placeholder return statement.");
            expect(wrapper_source.find("bool activates_adopted_return = false;") != std::string::npos,
                   "build host FLL wrapper should record the inactive return-activation flag.");
            expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
                   "build host FLL wrapper should record the placeholder-emission flag.");
            expect(wrapper_source.find("int fallback_int_value = -1;") != std::string::npos,
                   "build host FLL wrapper should record the placeholder fallback integer value.");
            expect(wrapper_source.find("int success_int_value = -1;") != std::string::npos,
                   "build host FLL wrapper should record the typed native success integer value.");
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
