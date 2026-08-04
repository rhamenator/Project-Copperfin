// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"

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
#include <dlfcn.h>
#include <sys/wait.h>
#else
#include <process.h>
#include <windows.h>
#endif

namespace {

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::ScopedTestLocaleCatalogDirectory;
using copperfin::test_support::set_env_value;

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

const copperfin::localization::LocalizedCatalog& build_host_catalog(const std::string& locale) {
    static const std::unordered_map<std::string, copperfin::localization::LocalizedCatalog> catalogs = {
        {"en-US", copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "en-US")},
        {"es-419", copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "es-419")},
        {"pt-BR", copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "pt-BR")},
        {"qps-ploc", copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "qps-ploc")}
    };
    return catalogs.at(locale);
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

std::string process_failure_detail(const ProcessResult& process) {
    std::string detail;
    if (!process.stdout_text.empty()) {
        detail += " stdout=" + process.stdout_text;
    }
    if (!process.stderr_text.empty()) {
        detail += " stderr=" + process.stderr_text;
    }
    return detail;
}

void expect_process_success(
    const ProcessResult& process,
    const std::string& message) {
    expect(
        process.exit_code == 0,
        process.exit_code == 0 ? message : message + "; exit=" +
            std::to_string(process.exit_code) + process_failure_detail(process));
}

void expect_process_output(
    const ProcessResult& process,
    const bool condition,
    const std::string& message) {
    expect(
        condition,
        condition ? message : message + process_failure_detail(process));
}

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    namespace fs = std::filesystem;

    const fs::path stdout_path = working_directory / "build_host_stdout.log";
    const fs::path stderr_path = working_directory / "build_host_stderr.log";
    const fs::path executable_hint(executable_path);
    const std::string command_executable =
        executable_hint.is_absolute() || executable_hint.has_parent_path()
            ? fs::absolute(executable_hint).string()
            : executable_path;

    std::string command = quote_command_argument(command_executable);
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
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind(prefix, 0U) == 0U) {
            return line.substr(prefix.size());
        }
    }
    return {};
}

void test_value_for_key_accepts_crlf_output() {
    const std::string output =
        "status: ok\r\n"
        "manifest.path: C:\\packages\\app.cfmanifest\r\n";
    expect(value_for_key(output, "manifest.path") == "C:\\packages\\app.cfmanifest",
           "build-host stdout parser should remove CRLF framing from machine-readable values");
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
        } else if (ch == '|') {
            escaped += "\\|";
        } else {
            escaped.push_back(ch);
        }
    }
    return escaped;
}

std::string unquote_manifest_value(const std::string& value) {
    std::string unescaped;
    unescaped.reserve(value.size());
    for (std::size_t index = 0U; index < value.size(); ++index) {
        if (value[index] != '\\' || index + 1U >= value.size()) {
            unescaped.push_back(value[index]);
            continue;
        }
        const char escaped = value[++index];
        if (escaped == '\\') {
            unescaped.push_back('\\');
        } else if (escaped == 'n') {
            unescaped.push_back('\n');
        } else if (escaped == 'r') {
            unescaped.push_back('\r');
        } else if (escaped == '|') {
            unescaped.push_back('|');
        } else {
            unescaped.push_back('\\');
            unescaped.push_back(escaped);
        }
    }
    return unescaped;
}

bool manifest_source_location_matches(
    const std::string& text,
    const std::string& key,
    const std::string& symbol,
    const std::filesystem::path& expected_path,
    std::size_t expected_line) {
    std::istringstream input(text);
    std::string line;
    const std::string prefix = key + "=" + symbol + "|";
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind(prefix, 0U) != 0U) {
            continue;
        }

        const std::string encoded_location = line.substr(prefix.size());
        const std::size_t separator = encoded_location.rfind('|');
        if (separator == std::string::npos ||
            encoded_location.substr(separator + 1U) != std::to_string(expected_line)) {
            continue;
        }

        const std::filesystem::path actual_path = unquote_manifest_value(
            encoded_location.substr(0U, separator));
        std::error_code equivalent_error;
        if (std::filesystem::equivalent(actual_path, expected_path, equivalent_error) &&
            !equivalent_error) {
            return true;
        }
        if (actual_path.lexically_normal() == expected_path.lexically_normal()) {
            return true;
        }
    }
    return false;
}

std::filesystem::path manifest_path_for_key(const std::string& text, const std::string& key) {
    return unquote_manifest_value(manifest_value_for_key(text, key));
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

#include "test_build_host_output_library_smoke.inl"
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

    expect_process_success(process, "build host should succeed for APP outputs");
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
        expect(manifest_text.find("primary_output_materialized=") == std::string::npos,
               "build host runtime manifest should omit the materialized APP primary output state");
        expect(manifest_text.find("extension_payload=" + quote_manifest_value(expected_output.string()) + "|") != std::string::npos,
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

    expect_process_success(process, "build host should succeed for FXP outputs");
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
        expect(manifest_text.find("primary_output_materialized=") == std::string::npos,
               "build host runtime manifest should omit the materialized FXP primary output state");
        expect(manifest_text.find("extension_payload=" + quote_manifest_value(expected_output.string()) + "|") != std::string::npos,
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

    write_text(temp_project_dir / "main.prg", "WAIT WINDOW 'host-resolution'\nRETURN\n");
    write_synthetic_executable_project(project_path, temp_project_dir, expected_output);

    {
        ScopedEnvironmentValue clear_runtime_host_env("COPPERFIN_RUNTIME_HOST_PATH");
        ScopedEnvironmentValue clear_license_path("COPPERFIN_LICENSE_PATH");

        const auto process = run_process_capture(
            temp_build_host.string(),
            {"build", "--project", project_path.string(), "--output-dir", output_dir.string()},
            temp_root);

        expect(process.exit_code == 0, "build host should resolve runtime host from executable directory");
        expect_process_output(
            process,
            process.stdout_text.find("status: ok") != std::string::npos,
            "runtime-host resolution smoke test should report status: ok");
        expect_process_output(
            process,
            process.stdout_text.find("output.kind: executable") != std::string::npos,
            "runtime-host resolution smoke test should build an executable output");
        expect_process_output(
            process,
            process.stdout_text.find("primary.output.materialized: true") != std::string::npos,
            "runtime-host resolution smoke test should materialize executable output");
        const fs::path manifest_path = value_for_key(process.stdout_text, "manifest.path");
        expect(!manifest_path.empty(), "runtime-host resolution smoke test should report manifest path");
        if (!manifest_path.empty()) {
            const std::string manifest_text = read_text(manifest_path);
            expect(manifest_text.find("runtime_host_sha256=") != std::string::npos,
                   "runtime-host resolution smoke test should persist runtime host digest");
            expect(manifest_text.find("primary_output_materialized=") == std::string::npos,
                   "runtime-host resolution smoke test manifest should omit primary-output materialization state");
        }
        expect(fs::exists(expected_output), "runtime-host fallback test should materialize requested executable");

        fs::remove_all(output_dir, ignored);
        fs::create_directories(output_dir);
        const fs::path caller_root = temp_root / "unrelated-caller";
        fs::create_directories(caller_root);
        const fs::path caller_runtime_host = caller_root / source_runtime_host.filename();
        write_text(caller_runtime_host, "caller-cwd-runtime-host-decoy\n");
        write_text(caller_root / "license.cflicense", "{\"caller\":true}\n");

        ScopedEnvironmentValue search_path("PATH", false);
        const std::string original_path = copperfin::test_support::getenv_value("PATH");
#if defined(_WIN32)
        ScopedEnvironmentValue path_extensions("PATHEXT", ".EXE;.COM;.BAT;.CMD");
        constexpr char path_separator = ';';
        const std::string launch_name = temp_build_host.stem().string();
#else
        constexpr char path_separator = ':';
        const std::string launch_name = temp_build_host.filename().string();
#endif
        search_path.set(
            temp_bundle.string() +
            (original_path.empty()
                 ? std::string()
                 : std::string(1U, path_separator) + original_path));

        const auto path_process = run_process_capture(
            launch_name,
            {"build", "--project", project_path.string(), "--output-dir", output_dir.string()},
            caller_root);

        expect_process_output(
            path_process,
            path_process.exit_code == 0,
            "#4017: PATH-launched build host should resolve its deployed runtime-host sibling");
        expect_process_output(
            path_process,
            path_process.stdout_text.find("status: ok") != std::string::npos,
            "#4017: PATH-launched sibling resolution should preserve invariant success status");
        const fs::path path_debug_manifest = value_for_key(
            path_process.stdout_text,
            "debug.manifest.path");
        expect(!path_debug_manifest.empty() &&
                   read_text(path_debug_manifest).find("license_state=\n") != std::string::npos,
               "#4900: PATH-launched package metadata should leave archived product-license state empty");
        const fs::path staged_runtime_host = expected_output.parent_path() / source_runtime_host.filename();
        expect(fs::exists(staged_runtime_host),
               "#4017: PATH-launched packaging should stage the runtime host");
        if (fs::exists(staged_runtime_host)) {
            expect(read_text(staged_runtime_host) == read_text(temp_runtime_host),
                   "#4017: PATH-launched packaging should stage the deployed runtime-host bytes");
        }

        const fs::path environment_runtime_host = temp_root / "environment-runtime-host.bin";
        const fs::path explicit_runtime_host = temp_root / "explicit-runtime-host.bin";
        write_text(environment_runtime_host, "environment-runtime-host\n");
        write_text(explicit_runtime_host, "explicit-runtime-host\n");

        fs::remove_all(output_dir, ignored);
        fs::create_directories(output_dir);
        clear_runtime_host_env.set(environment_runtime_host.string());
        const auto environment_process = run_process_capture(
            launch_name,
            {"build", "--project", project_path.string(), "--output-dir", output_dir.string()},
            caller_root);
        expect(environment_process.exit_code == 0,
               "#4017: COPPERFIN_RUNTIME_HOST_PATH should override sibling discovery");
        expect(fs::exists(staged_runtime_host) &&
                   read_text(staged_runtime_host) == read_text(environment_runtime_host),
               "#4017: the environment runtime-host override should provide staged bytes");

        fs::remove_all(output_dir, ignored);
        fs::create_directories(output_dir);
        const auto explicit_process = run_process_capture(
            launch_name,
            {
                "build",
                "--project",
                project_path.string(),
                "--output-dir",
                output_dir.string(),
                "--runtime-host",
                explicit_runtime_host.string()
            },
            caller_root);
        expect(explicit_process.exit_code == 0,
               "#4017: --runtime-host should override COPPERFIN_RUNTIME_HOST_PATH");
        expect(fs::exists(staged_runtime_host) &&
                   read_text(staged_runtime_host) == read_text(explicit_runtime_host),
               "#4017: the explicit runtime-host override should provide staged bytes");
    }

    fs::remove_all(temp_root, ignored);
}

void run_emit_dotnet_launcher_fallback_smoke(const std::string& build_host_path) {
#if defined(_WIN32)
    (void)build_host_path;
#else
    namespace fs = std::filesystem;

    expect(fs::exists(build_host_path), "build host executable should exist before running dotnet-launcher fallback smoke test");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_build_host_dotnet_fallback_smoke";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path project_path = project_dir / "dotnetfallback.pjx";
    const fs::path expected_output = output_dir / "DotNetFallback" / "DotNetFallback.exe";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);

    // Keep the fallback assertion independent of whichever dotnet installation owns the test host.
    ScopedEnvironmentValue unavailable_dotnet_path("PATH", temp_root.string());

    write_text(project_dir / "main.prg", "WAIT WINDOW 'dotnet-fallback'\nRETURN\n");
    write_synthetic_executable_project(project_path, project_dir, expected_output, "DotNetFallback");

    const auto process = run_process_capture(
        build_host_path,
        {
            "build",
            "--project",
            project_path.string(),
            "--output-dir",
            output_dir.string(),
            "--emit-dotnet-launcher"
        },
        temp_root);

    expect_process_output(
        process,
        process.exit_code == 0,
        "build host should fall back to native packaging for POSIX --emit-dotnet-launcher requests");
    expect_process_output(
        process,
        process.stdout_text.find("status: ok") != std::string::npos,
        "dotnet-launcher fallback smoke should preserve machine-readable success status");
    expect_process_output(
        process,
        process.stdout_text.find("output.kind: executable") != std::string::npos,
        "dotnet-launcher fallback smoke should preserve executable output kind");
    expect_process_output(
        process,
        process.stdout_text.find("primary.output.materialized: true") != std::string::npos,
        "dotnet-launcher fallback smoke should still materialize a primary output");
    expect(fs::exists(expected_output),
           "dotnet-launcher fallback smoke should materialize the native executable output");

    const fs::path manifest_path = value_for_key(process.stdout_text, "manifest.path");
    const fs::path debug_manifest_path = value_for_key(process.stdout_text, "debug.manifest.path");
    expect(!manifest_path.empty(),
           "dotnet-launcher fallback smoke should report a runtime manifest path");
    expect(!debug_manifest_path.empty(),
           "dotnet-launcher fallback smoke should report a debug manifest path");
    if (!manifest_path.empty()) {
        const std::string manifest_text = read_text(manifest_path);
        expect(manifest_text.find("launcher_mode=") == std::string::npos,
               "dotnet-launcher fallback runtime manifest should omit launcher mode from the execution contract");
        expect(manifest_text.find("launcher_fallback=") == std::string::npos,
               "dotnet-launcher fallback runtime manifest should omit launcher fallback from the execution contract");
        expect(lines_with_prefix(manifest_text, "feature_flag=").empty(),
               "dotnet-launcher fallback runtime manifest should omit feature-flag inventory");
    }
    if (!debug_manifest_path.empty()) {
        const std::string debug_manifest_text = read_text(debug_manifest_path);
        expect(debug_manifest_text.find("launcher_mode=native_runtime_host") != std::string::npos,
               "dotnet-launcher fallback debug manifest should record native runtime host mode");
        expect(debug_manifest_text.find("launcher_fallback=dotnet_output_unavailable") != std::string::npos,
               "dotnet-launcher fallback debug manifest should record the unavailability fallback reason");
        expect(debug_manifest_text.find("feature_flag=launcher.dotnet.requested|true|rollout") != std::string::npos,
               "dotnet-launcher fallback debug manifest should preserve the requested launcher feature flag");
        expect(debug_manifest_text.find("feature_flag=launcher.dotnet.active|false|host_compatibility") != std::string::npos,
               "dotnet-launcher fallback debug manifest should record the inactive launcher feature flag");
    }

    fs::remove_all(temp_root, ignored);
#endif
}

void run_build_host_localized_usage_smoke(const std::string& build_host_path) {
    namespace fs = std::filesystem;

    expect(fs::exists(build_host_path), "build host executable should exist before running localized usage smoke test");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_build_host_localized_usage";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    {
        ScopedEnvironmentValue clear_locale("COPPERFIN_LOCALE");
        ScopedTestLocaleCatalogDirectory locale_dir;
        const auto process = run_process_capture(build_host_path, {}, temp_root);
        expect(process.exit_code == 2, "build host without a build command should fail usage validation");
        expect(process.stdout_text.find(
                   "Usage: copperfin_build_host build --project <path-to-pjx> --output-dir <directory>") !=
                   std::string::npos,
               "default build host usage should preserve the en-US CLI text");
        expect(process.stdout_text.find("--license-status") == std::string::npos,
               "#4900: default build host usage should not advertise archived product licensing");
        expect(process.stdout_text.find("--configuration debug|release") != std::string::npos,
               "default build host usage should preserve configuration tokens");

        const auto invalid_configuration = run_process_capture(
            build_host_path, {"build", "--configuration", "releaes"}, temp_root);
        expect(invalid_configuration.exit_code == 2,
               "invalid build configuration should fail before package planning");
        expect(invalid_configuration.stdout_text.find("status: error") != std::string::npos &&
                   invalid_configuration.stdout_text.find(
                       "Invalid build configuration: releaes. Expected debug or release.") != std::string::npos,
               "invalid build configuration should use the localized validation diagnostic");

        const auto empty_configuration = run_process_capture(
            build_host_path, {"build", "--configuration", ""}, temp_root);
        expect(empty_configuration.exit_code == 2 &&
                   empty_configuration.stdout_text.find("Invalid build configuration:") != std::string::npos,
               "empty build configuration should fail explicitly");

        for (const char* configuration : {" DEBUG ", " RELEASE "}) {
            const auto normalized_configuration = run_process_capture(
                build_host_path, {"build", "--configuration", configuration}, temp_root);
            expect(normalized_configuration.exit_code == 2 &&
                       normalized_configuration.stdout_text.find("Invalid build configuration:") == std::string::npos &&
                       normalized_configuration.stdout_text.find("--project and --output-dir are required.") != std::string::npos,
                   "case-insensitive whitespace-padded build configuration should be accepted before required-option validation");
        }
    }

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        ScopedTestLocaleCatalogDirectory locale_dir;
        set_env_value("COPPERFIN_LOCALE", "es-419", true);

        const auto process = run_process_capture(build_host_path, {}, temp_root);
        expect(process.exit_code == 2, "Spanish placeholder build host usage should still fail usage validation");
        expect(process.stdout_text.find("Uso: copperfin_build_host build --project <path-to-pjx> --output-dir <directory>") !=
                   std::string::npos,
               "Spanish placeholder build host usage should resolve through the es-419 catalog");
        expect(process.stdout_text.find("--license-status") == std::string::npos,
               "#4900: Spanish build host usage should not advertise archived product licensing");
        expect(process.stdout_text.find("Usage: copperfin_build_host") == std::string::npos,
               "Spanish placeholder build host usage should not fall back to raw English prose");
        expect(process.stdout_text.find("--configuration debug|release") != std::string::npos,
               "Spanish placeholder build host usage should preserve invariant configuration tokens");
    }

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        ScopedTestLocaleCatalogDirectory locale_dir;
        set_env_value("COPPERFIN_LOCALE", "pt-BR", true);

        const auto process = run_process_capture(build_host_path, {"build", "--project"}, temp_root);
        expect(process.exit_code == 2, "Portuguese placeholder build host errors should still use the original exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "Portuguese placeholder build host errors should preserve machine-readable status");
        expect(process.stdout_text.find("Argumento desconhecido ou incompleto: --project") != std::string::npos,
               "Portuguese placeholder build host errors should resolve through the pt-BR catalog");
        expect(process.stdout_text.find("erro: ") != std::string::npos,
               "Portuguese placeholder build host errors should localize the error prefix");
        expect(process.stdout_text.find("Unknown or incomplete argument") == std::string::npos,
               "Portuguese placeholder build host errors should not fall back to raw English prose");
    }

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        ScopedTestLocaleCatalogDirectory locale_dir;
        set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

        const auto process = run_process_capture(build_host_path, {}, temp_root);
        expect(process.exit_code == 2, "pseudo-localized build host usage should still fail usage validation");
        expect(process.stdout_text.find("[!! ") != std::string::npos,
               "pseudo-localized build host usage should decorate prose");
        expect(process.stdout_text.find("copperfin_build_host") != std::string::npos,
               "pseudo-localized build host usage should preserve the command name");
        expect(process.stdout_text.find("--license-status") == std::string::npos,
               "#4900: pseudo-localized build host usage should not advertise archived product licensing");
        expect(process.stdout_text.find("--project") != std::string::npos,
               "pseudo-localized build host usage should preserve CLI flags");
        expect(process.stdout_text.find("debug|release") != std::string::npos,
               "pseudo-localized build host usage should preserve invariant configuration values");

        const auto error_process = run_process_capture(build_host_path, {"build", "--project"}, temp_root);
        expect(error_process.exit_code == 2,
               "pseudo-localized build host command errors should still use the original exit code");
        expect(error_process.stdout_text.find("status: error") != std::string::npos,
               "pseudo-localized build host command errors should preserve machine-readable status");
        const std::string pseudo_error_prefix = build_host_catalog("qps-ploc").translate("BuildHost.Prefix.Error");
        expect(error_process.stdout_text.find(pseudo_error_prefix) != std::string::npos,
               "pseudo-localized build host command errors should route the error prefix through qps-ploc");
        expect(error_process.stdout_text.find("[!! ") != std::string::npos,
               "pseudo-localized build host command errors should decorate prose");
        expect(error_process.stdout_text.find("--project") != std::string::npos,
               "pseudo-localized build host command errors should preserve the invalid CLI flag");
        expect(error_process.stdout_text.find("error: Unknown or incomplete argument") == std::string::npos,
               "pseudo-localized build host command errors should not fall back to the raw English prefixed error");
    }

    {
        const auto process = run_process_capture(build_host_path, {"--license-status"}, temp_root);
        expect(process.exit_code == 2, "#4900: normalized license-status flag should be inactive");
        expect(process.stdout_text.find("state: ") == std::string::npos,
               "#4900: inactive license-status flag should not print product-license state");
    }

    {
        const auto process = run_process_capture(build_host_path, {"license-status"}, temp_root);
        expect(process.exit_code == 2, "#4900: legacy positional license-status command should be inactive");
        expect(process.stdout_text.find("state: ") == std::string::npos,
               "#4900: inactive legacy command should not print product-license state");
    }

    {
        const std::filesystem::path project_dir = temp_root / "warning_project";
        const std::filesystem::path output_dir = temp_root / "warning_output";
        const std::filesystem::path project_path = project_dir / "warningdemo.pjx";
        std::error_code ignored_warning_setup;
        fs::remove_all(project_dir, ignored_warning_setup);
        fs::remove_all(output_dir, ignored_warning_setup);
        fs::create_directories(project_dir);
        fs::create_directories(output_dir);

        write_text(project_dir / "librarymain.prg", "RETURN\n");
        write_text(project_dir / "helper.prg", "RETURN\n");
        write_synthetic_project(project_path, project_dir, output_dir / "WarningDemo.dll");

        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        ScopedTestLocaleCatalogDirectory locale_dir;
        set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

        const auto process = run_process_capture(
            build_host_path,
            {"build", "--project", project_path.string(), "--output-dir", output_dir.string()},
            temp_root);
        const std::string pseudo_warning_prefix = build_host_catalog("qps-ploc").translate("BuildHost.Prefix.Warning");
        expect(process.exit_code == 0,
               "pseudo-localized build host warning-path builds should still succeed");
        expect(process.stdout_text.find("status: ok") != std::string::npos,
               "pseudo-localized build host warning-path builds should preserve machine-readable status");
        expect(process.stdout_text.find(pseudo_warning_prefix) != std::string::npos,
               "pseudo-localized build host warnings should route the warning prefix through qps-ploc");
        expect(process.stdout_text.find("warning: No PRG routine exports were discovered") == std::string::npos,
               "pseudo-localized build host warnings should not fall back to the raw English prefixed warning");
    }

    fs::remove_all(temp_root, ignored);
}

void run_build_host_localized_usage_path_search_smoke(const std::string& build_host_path) {
    namespace fs = std::filesystem;

    expect(fs::exists(build_host_path), "build host executable should exist before running PATH-localized usage smoke test");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_build_host_localized_usage_path_search";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    const fs::path nested_working_directory = temp_root / "cwd" / "nested";
    const fs::path deployed_executable_directory = temp_root / "bin";
    const fs::path deployed_locale_root = temp_root / "share" / "copperfin" / "locales";
    fs::create_directories(nested_working_directory);
    fs::create_directories(deployed_executable_directory);
    fs::create_directories(deployed_locale_root.parent_path());

    const fs::path resolved_build_host_path = fs::absolute(build_host_path);
    const fs::path deployed_build_host_path =
        deployed_executable_directory / resolved_build_host_path.filename();
    fs::copy_file(
        resolved_build_host_path,
        deployed_build_host_path,
        fs::copy_options::overwrite_existing);
    fs::copy(
        copperfin::test_support::configured_test_locale_catalog_dir(),
        deployed_locale_root,
        fs::copy_options::recursive | fs::copy_options::overwrite_existing);

    const std::string executable_name = deployed_build_host_path.filename().string();
    const std::string original_path = copperfin::test_support::getenv_value("PATH");
#if defined(_WIN32)
    const char path_separator = ';';
#else
    const char path_separator = ':';
#endif
    const std::string seeded_path =
        deployed_build_host_path.parent_path().string() +
        (original_path.empty() ? std::string() : std::string(1U, path_separator) + original_path);

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        ScopedEnvironmentValue clear_locale_dir("COPPERFIN_LOCALE_DIR");
        ScopedEnvironmentValue search_path("PATH", false);
        set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
        search_path.set(seeded_path);

        const auto process = run_process_capture(executable_name, {}, nested_working_directory);
        expect(process.exit_code == 2,
               "PATH-launched pseudo-localized build host usage should still fail usage validation");
        expect(process.stdout_text.find("[!! ") != std::string::npos,
               "PATH-launched pseudo-localized build host usage should decorate prose through the located catalogs");
        expect(process.stdout_text.find("BuildHost.Usage") == std::string::npos,
               "PATH-launched build host usage should not leak the raw usage catalog key");
        expect(process.stdout_text.find("BuildHost.Usage.LicenseStatus") == std::string::npos,
               "PATH-launched build host usage should not leak the raw license-status catalog key");
        expect(process.stdout_text.find("--license-status") == std::string::npos,
               "#4900: PATH-launched build host usage should hide archived product licensing");
    }

    fs::remove_all(temp_root, ignored);
}

void run_build_host_explicit_locale_manifest_smoke(const std::string& build_host_path) {
    namespace fs = std::filesystem;

    expect(fs::exists(build_host_path), "build host executable should exist before running explicit-locale manifest smoke test");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_build_host_explicit_locale_manifest";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path project_path = project_dir / "explicitlocale.pjx";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_synthetic_executable_project(project_path, project_dir, output_dir / "ExplicitLocale.dll", "ExplicitLocale");

    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    ScopedTestLocaleCatalogDirectory locale_dir;
    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    const auto process = run_process_capture(
        build_host_path,
        {
            "build",
            "--locale", "qps-ploc",
            "--project", project_path.string(),
            "--output-dir", output_dir.string()
        },
        temp_root);

    expect_process_success(process, "build host explicit-locale manifest smoke should succeed");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "build host explicit-locale manifest smoke should preserve machine-readable status");
    expect(process.stdout_text.find("output.kind: dll") != std::string::npos,
           "build host explicit-locale manifest smoke should preserve invariant output kind");

    const fs::path manifest_path = value_for_key(process.stdout_text, "manifest.path");
    const std::string manifest_text = manifest_path.empty() ? std::string{} : read_text(manifest_path);
    const std::string pseudo_security_mode = build_host_catalog("qps-ploc").translate("Security.Profile.Mode");
    expect(manifest_text.find("project_title=ExplicitLocale") != std::string::npos,
           "build host explicit-locale manifest should preserve the invariant project title");
    expect(manifest_text.find("security_mode=" + quote_manifest_value(pseudo_security_mode)) != std::string::npos,
           "build host explicit-locale manifest should use the selected catalog for security metadata");
    expect(manifest_text.find("security_mode=" + quote_manifest_value(
               build_host_catalog("en-US").translate("Security.Profile.Mode"))) == std::string::npos,
           "build host explicit-locale manifest should not fall back to the environment catalog");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_build_host_output <path-to-build-host>\n";
        return EXIT_FAILURE;
    }

    test_value_for_key_accepts_crlf_output();
    run_library_build_host_smoke(argv[1], "dll");
    run_library_build_host_smoke(argv[1], "fll");
    run_app_build_host_smoke(argv[1]);
    run_fxp_build_host_smoke(argv[1]);
    run_default_runtime_host_resolution_smoke(argv[1]);
    run_emit_dotnet_launcher_fallback_smoke(argv[1]);
    run_build_host_localized_usage_smoke(argv[1]);
    run_build_host_localized_usage_path_search_smoke(argv[1]);
    run_build_host_explicit_locale_manifest_smoke(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
