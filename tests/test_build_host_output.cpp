#include "copperfin/vfp/dbf_table.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
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

    write_text(project_dir / "librarymain.prg", "PROCEDURE InitLibrary\nRETURN\nENDPROC\n");
    write_text(project_dir / "helper.prg", "FUNCTION AddNumbers\nRETURN 1\nENDFUNC\n");
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
    expect(!manifest_path.empty(), "build host should report a manifest path for " + extension + " outputs");
    if (!manifest_path.empty()) {
        const std::string manifest_text = read_text(manifest_path);
        expect(manifest_text.find("primary_output_materialized=true") != std::string::npos,
               "build host manifest should record a materialized primary output for " + extension + " outputs");
        expect(manifest_text.find("extension_payload=" + expected_output.string() + "|") != std::string::npos,
               "build host manifest should record the built primary output as an extension payload for " + extension + " outputs");
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

        if (extension == "fll") {
            const fs::path fll_api_manifest_path = value_for_key(process.stdout_text, "fll.api.manifest");
            const std::set<std::string> declared_api_symbols = read_fll_api_declared_symbols(fll_api_manifest_path);
            expect(exported_symbols == declared_api_symbols,
                   "build host should preserve the API-manifest export contract for fll outputs");
        }
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

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
