#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/runtime/xasset_methods.h"
#include "prg_engine_internal.h"
#include "copperfin/security/sha256.h"

#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <unordered_set>

namespace copperfin::runtime {

namespace {

std::string sanitize_file_name(const std::string& value) {
    std::string sanitized;
    sanitized.reserve(value.size());
    for (const char ch : value) {
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' || ch == '_') {
            sanitized.push_back(ch);
        } else {
            sanitized.push_back('_');
        }
    }
    return sanitized.empty() ? "copperfin_app" : sanitized;
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

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

BuildOutputKind parse_build_output_kind(const std::string& value) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    if (normalized == "dll") {
        return BuildOutputKind::dll;
    }
    if (normalized == "app") {
        return BuildOutputKind::app;
    }
    if (normalized == "fll") {
        return BuildOutputKind::fll;
    }
    if (normalized == "fxp") {
        return BuildOutputKind::fxp;
    }
    if (normalized == "ocx") {
        return BuildOutputKind::ocx;
    }
    if (normalized == "executable") {
        return BuildOutputKind::executable;
    }
    return BuildOutputKind::unknown;
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

std::vector<std::string> unique_non_empty_paths_preserve_order(std::initializer_list<std::string> values) {
    std::vector<std::string> normalized_values;
    normalized_values.reserve(values.size());
    std::unordered_set<std::string> seen;
    for (const std::string& value : values) {
        if (value.empty()) {
            continue;
        }
        const std::string normalized = std::filesystem::path(value).lexically_normal().string();
        if (normalized.empty() || !seen.insert(normalized).second) {
            continue;
        }
        normalized_values.push_back(normalized);
    }
    return normalized_values;
}

std::string dotnet_parity_tier_name(copperfin::platform::DotNetParityTier tier) {
    switch (tier) {
        case copperfin::platform::DotNetParityTier::exact:
            return "exact";
        case copperfin::platform::DotNetParityTier::adapted:
            return "adapted";
        case copperfin::platform::DotNetParityTier::intentionally_not_supported:
            return "intentionally_not_supported";
        default:
            return "unknown";
    }
}

bool write_text_file(const std::filesystem::path& path, const std::string& contents, std::string& error) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        error = "Unable to create file: " + path.string();
        return false;
    }

    output << contents;
    if (!output.good()) {
        error = "Unable to write file: " + path.string();
        return false;
    }

    return true;
}

std::string read_text_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream stream;
    stream << input.rdbuf();
    return stream.str();
}

std::string read_binary_file(const std::filesystem::path& path, std::string& error) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        error = "Unable to open file: " + path.string();
        return {};
    }

    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string hex_encode_bytes(const std::string& bytes) {
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (const unsigned char byte : bytes) {
        stream << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return stream.str();
}

bool append_runtime_artifact_digest(
    std::vector<RuntimeArtifactDigest>& digests,
    const std::string& path,
    std::string& error) {
    if (trim_copy(path).empty() || !std::filesystem::exists(path)) {
        return true;
    }

    const auto digest = security::sha256_hex_for_file(path);
    if (!digest.ok) {
        error = digest.error;
        return false;
    }

    const auto existing = std::find_if(digests.begin(), digests.end(), [&](const RuntimeArtifactDigest& entry) {
        return entry.path == path;
    });
    if (existing != digests.end()) {
        existing->sha256 = digest.hex_digest;
        return true;
    }

    digests.push_back({
        .path = path,
        .sha256 = digest.hex_digest
    });
    return true;
}

bool is_library_output_kind(const BuildOutputKind output_kind) {
    return output_kind == BuildOutputKind::dll ||
        output_kind == BuildOutputKind::fll ||
        output_kind == BuildOutputKind::ocx;
}

bool is_native_host_output_kind(const BuildOutputKind output_kind) {
    return output_kind == BuildOutputKind::executable ||
        output_kind == BuildOutputKind::unknown;
}

constexpr std::string_view kFllLoaderEntrypoint = "FoxInfo";
constexpr std::string_view kFllRegistrationSymbol = "_FoxTable";
constexpr std::string_view kFllCallableSignature = "ParamBlk*";
constexpr std::string_view kFllDefaultReturnHelper = "_RetInt";
constexpr std::string_view kVfpLibraryCallableConvention = "vfp_declare_default";

std::string resolve_output_file_name(const studio::StudioProjectWorkspace& workspace, const std::string& project_title) {
    const std::filesystem::path configured_output(workspace.build_plan.output_path);
    const std::string file_name = configured_output.filename().string();
    if (!trim_copy(file_name).empty()) {
        return file_name;
    }
    return sanitize_file_name(project_title) + ".exe";
}

BuildOutputKind infer_build_output_kind_from_output_path(const std::string& output_path) {
    const std::string extension = lowercase_copy(trim_copy(std::filesystem::path(output_path).extension().string()));
    if (extension == ".dll") {
        return BuildOutputKind::dll;
    }
    if (extension == ".app") {
        return BuildOutputKind::app;
    }
    if (extension == ".fll") {
        return BuildOutputKind::fll;
    }
    if (extension == ".fxp") {
        return BuildOutputKind::fxp;
    }
    if (extension == ".ocx") {
        return BuildOutputKind::ocx;
    }
    if (extension == ".exe") {
        return BuildOutputKind::executable;
    }
    return BuildOutputKind::unknown;
}

std::string normalize_export_symbol(std::string value) {
    value = trim_copy(std::move(value));
    const std::size_t whitespace = value.find_first_of(" \t(");
    if (whitespace != std::string::npos) {
        value = trim_copy(value.substr(0U, whitespace));
    }
    return value;
}

std::string json_escape(std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }
    return escaped;
}

std::vector<std::string> collect_library_exported_symbols(const RuntimePackagePlan& plan) {
    std::vector<std::string> exported_symbols;
    std::unordered_set<std::string> seen;

    for (const auto& asset : plan.assets) {
        if (asset.excluded || !asset.exists) {
            continue;
        }

        if (lowercase_copy(std::filesystem::path(asset.source_path).extension().string()) != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }
            exported_symbols.push_back(export_name);
        }
    }

    return exported_symbols;
}

std::map<std::string, std::size_t> collect_library_export_parameter_counts(const RuntimePackagePlan& plan) {
    std::map<std::string, std::size_t> parameter_counts;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

        const std::string extension = lowercase_copy(std::filesystem::path(asset.source_path).extension().string());
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            std::size_t parameter_count = 0U;
            for (const auto& statement : routine.statements) {
                if (statement.kind != StatementKind::parameters_declaration &&
                    statement.kind != StatementKind::lparameters_declaration) {
                    continue;
                }

                parameter_count = statement.names.size();
                break;
            }

            parameter_counts.emplace(export_name, parameter_count);
        }
    }

    return parameter_counts;
}

std::string build_routine_kind_name(const RoutineKind kind) {
    switch (kind) {
        case RoutineKind::procedure:
            return "procedure";
        case RoutineKind::function:
            return "function";
        case RoutineKind::main:
        default:
            return "main";
    }
}

std::string extract_declared_parameter_name(const std::string& raw_name) {
    std::string parameter_name = trim_copy(raw_name);
    if (!parameter_name.empty() && parameter_name.front() == '@') {
        parameter_name.erase(parameter_name.begin());
    }
    const std::size_t equals = parameter_name.find('=');
    if (equals != std::string::npos) {
        parameter_name = trim_copy(parameter_name.substr(0U, equals));
    }
    return parameter_name;
}

std::string sanitize_cpp_identifier(const std::string& value, const std::size_t fallback_index) {
    std::string sanitized;
    sanitized.reserve(value.size() + 8U);
    for (const char ch : value) {
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '_') {
            sanitized.push_back(ch);
        } else {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        sanitized = "arg" + std::to_string(fallback_index + 1U);
    }
    if ((sanitized.front() >= '0' && sanitized.front() <= '9')) {
        sanitized.insert(sanitized.begin(), '_');
    }
    return sanitized;
}

std::map<std::string, std::vector<std::string>> collect_library_export_parameter_names(const RuntimePackagePlan& plan) {
    std::map<std::string, std::vector<std::string>> parameter_names;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

        const std::string extension = lowercase_copy(std::filesystem::path(asset.source_path).extension().string());
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            std::vector<std::string> names;
            for (const auto& statement : routine.statements) {
                if (statement.kind != StatementKind::parameters_declaration &&
                    statement.kind != StatementKind::lparameters_declaration) {
                    continue;
                }

                names.reserve(statement.names.size());
                for (std::size_t index = 0; index < statement.names.size(); ++index) {
                    names.push_back(sanitize_cpp_identifier(
                        extract_declared_parameter_name(statement.names[index]),
                        index));
                }
                break;
            }

            parameter_names.emplace(export_name, std::move(names));
        }
    }

    return parameter_names;
}

std::map<std::string, std::string> collect_library_export_parameter_declaration_kinds(const RuntimePackagePlan& plan) {
    std::map<std::string, std::string> declaration_kinds;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

        const std::string extension = lowercase_copy(std::filesystem::path(asset.source_path).extension().string());
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            std::string declaration_kind;
            for (const auto& statement : routine.statements) {
                if (statement.kind == StatementKind::parameters_declaration) {
                    declaration_kind = "parameters";
                    break;
                }
                if (statement.kind == StatementKind::lparameters_declaration) {
                    declaration_kind = "lparameters";
                    break;
                }
            }

            declaration_kinds.emplace(export_name, std::move(declaration_kind));
        }
    }

    return declaration_kinds;
}

std::map<std::string, std::string> collect_library_export_routine_kinds(const RuntimePackagePlan& plan) {
    std::map<std::string, std::string> routine_kinds;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

        const std::string extension = lowercase_copy(std::filesystem::path(asset.source_path).extension().string());
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            routine_kinds.emplace(export_name, build_routine_kind_name(routine.kind));
        }
    }

    return routine_kinds;
}

std::map<std::string, SourceLocation> collect_library_export_routine_locations(const RuntimePackagePlan& plan) {
    std::map<std::string, SourceLocation> routine_locations;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

        const std::string extension = lowercase_copy(std::filesystem::path(asset.source_path).extension().string());
        if (extension != ".prg") {
            continue;
        }

        const Program program = parse_program(asset.source_path);
        for (const auto& [_, routine] : program.routines) {
            const std::string export_name = normalize_export_symbol(routine.name);
            if (export_name.empty()) {
                continue;
            }

            const std::string normalized = lowercase_copy(export_name);
            if (!seen.insert(normalized).second) {
                continue;
            }

            routine_locations.emplace(export_name, routine.declaration_location);
        }
    }

    return routine_locations;
}

std::string build_placeholder_int_parameter_list(const std::vector<std::string>& parameter_names) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < parameter_names.size(); ++index) {
        if (index > 0U) {
            stream << ", ";
        }
        stream << "int " << sanitize_cpp_identifier(parameter_names[index], index);
    }
    return stream.str();
}

std::string build_manifest_parameter_names(const std::vector<std::string>& parameter_names) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < parameter_names.size(); ++index) {
        if (index > 0U) {
            stream << "|";
        }
        stream << quote_manifest_value(parameter_names[index]);
    }
    return stream.str();
}

std::string build_manifest_source_location(const SourceLocation& location) {
    std::ostringstream stream;
    stream << quote_manifest_value(location.file_path) << "|" << location.line;
    return stream.str();
}

std::string build_module_definition_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const std::string output_stem =
        std::filesystem::path(plan.launcher_output_path).stem().string();
    stream << "LIBRARY " << output_stem << "\n";
    stream << "EXPORTS\n";
    for (const auto& symbol : plan.exported_symbols) {
        stream << "    " << symbol << "\n";
    }
    if (plan.output_kind == BuildOutputKind::fll) {
        stream << "    " << kFllLoaderEntrypoint << "\n";
        stream << "    " << kFllRegistrationSymbol << "\n";
    }
    return stream.str();
}

std::string build_native_wrapper_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "// Generated Copperfin native wrapper scaffold\n";
    stream << "// This is an honest bridge scaffold, not a finished FoxPro/VFP-compatible runtime wrapper.\n";
    stream << "#include <cstdint>\n";
    stream << "#include <filesystem>\n";
    stream << "#include <fstream>\n";
    stream << "#include <sstream>\n";
    stream << "#include <string>\n";
    stream << "#include <vector>\n";
    stream << "#if defined(_WIN32)\n";
    stream << "#include <windows.h>\n";
    stream << "#define COPPERFIN_EXPORT extern \"C\" __declspec(dllexport)\n";
    stream << "#else\n";
    stream << "#include <dlfcn.h>\n";
    stream << "#define COPPERFIN_EXPORT extern \"C\" __attribute__((visibility(\"default\")))\n";
    stream << "#endif\n\n";
    stream << "static std::filesystem::path copperfin_wrapper_module_path(void* symbol_address) {\n";
    stream << "#if defined(_WIN32)\n";
    stream << "    HMODULE module = nullptr;\n";
    stream << "    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,\n";
    stream << "                            reinterpret_cast<LPCSTR>(symbol_address),\n";
    stream << "                            &module) || module == nullptr) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    char buffer[MAX_PATH] = {};\n";
    stream << "    const DWORD length = GetModuleFileNameA(module, buffer, MAX_PATH);\n";
    stream << "    if (length == 0U) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    return std::filesystem::path(std::string(buffer, buffer + length)).lexically_normal();\n";
    stream << "#else\n";
    stream << "    Dl_info info{};\n";
    stream << "    if (dladdr(symbol_address, &info) == 0 || info.dli_fname == nullptr) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    return std::filesystem::path(info.dli_fname).lexically_normal();\n";
    stream << "#endif\n";
    stream << "}\n\n";
    stream << "static std::filesystem::path copperfin_wrapper_module_directory(void* symbol_address) {\n";
    stream << "    const auto module_path = copperfin_wrapper_module_path(symbol_address);\n";
    stream << "    return module_path.empty() ? std::filesystem::path{} : module_path.parent_path();\n";
    stream << "}\n\n";
    stream << "static std::filesystem::path copperfin_runtime_manifest_path(void* symbol_address) {\n";
    stream << "    return copperfin_wrapper_module_directory(symbol_address) / \"app.cfmanifest\";\n";
    stream << "}\n\n";
    stream << "static std::filesystem::path copperfin_runtime_host_path(void* symbol_address) {\n";
    stream << "#if defined(_WIN32)\n";
    stream << "    return copperfin_wrapper_module_directory(symbol_address) / \"copperfin_runtime_host.exe\";\n";
    stream << "#else\n";
    stream << "    return copperfin_wrapper_module_directory(symbol_address) / \"copperfin_runtime_host\";\n";
    stream << "#endif\n";
    stream << "}\n\n";
    stream << "struct CopperfinRuntimeBridgeDescriptor {\n";
    stream << "    const char* export_name;\n";
    stream << "    const char* routine_kind;\n";
    stream << "    const char* source_path;\n";
    stream << "    unsigned int source_line;\n";
    stream << "    const char* parameter_declaration_kind;\n";
    stream << "    const char* parameter_names;\n";
    stream << "    unsigned int parameter_count;\n";
    stream << "    std::filesystem::path manifest_path;\n";
    stream << "    std::filesystem::path runtime_host_path;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeInvocation {\n";
    stream << "    CopperfinRuntimeBridgeDescriptor descriptor;\n";
    stream << "    std::vector<std::string> arguments;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeParameter {\n";
    stream << "    std::string parameter_name;\n";
    stream << "    std::string value_representation;\n";
    stream << "    std::string call_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeCall {\n";
    stream << "    CopperfinRuntimeBridgeInvocation invocation;\n";
    stream << "    std::vector<CopperfinRuntimeBridgeParameter> parameters;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturn {\n";
    stream << "    std::string value_representation;\n";
    stream << "    std::string return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeResult {\n";
    stream << "    CopperfinRuntimeBridgeCall call;\n";
    stream << "    CopperfinRuntimeBridgeReturn return_binding;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeEnvironmentVariable {\n";
    stream << "    std::string name;\n";
    stream << "    std::string value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeLaunchPlan {\n";
    stream << "    CopperfinRuntimeBridgeResult result;\n";
    stream << "    std::filesystem::path working_directory;\n";
    stream << "    std::vector<CopperfinRuntimeBridgeEnvironmentVariable> environment;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeObservationPlan {\n";
    stream << "    CopperfinRuntimeBridgeLaunchPlan launch_plan;\n";
    stream << "    std::filesystem::path stdout_log_path;\n";
    stream << "    std::filesystem::path stderr_log_path;\n";
    stream << "    int expected_exit_code = 0;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeExecutionPlan {\n";
    stream << "    CopperfinRuntimeBridgeObservationPlan observation_plan;\n";
    stream << "    std::filesystem::path executable_path;\n";
    stream << "    std::vector<std::string> arguments;\n";
    stream << "    bool capture_stdout = true;\n";
    stream << "    bool capture_stderr = true;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeTransportPlan {\n";
    stream << "    CopperfinRuntimeBridgeExecutionPlan execution_plan;\n";
    stream << "    std::filesystem::path request_path;\n";
    stream << "    std::filesystem::path response_path;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeSerializationPlan {\n";
    stream << "    CopperfinRuntimeBridgeTransportPlan transport_plan;\n";
    stream << "    std::string request_media_type;\n";
    stream << "    std::string response_media_type;\n";
    stream << "    std::string schema_version;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeDispatchPlan {\n";
    stream << "    CopperfinRuntimeBridgeSerializationPlan serialization_plan;\n";
    stream << "    std::vector<std::string> arguments;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeDispatchExecution {\n";
    stream << "    std::filesystem::path executable_path;\n";
    stream << "    std::vector<std::string> arguments;\n";
    stream << "    std::filesystem::path working_directory;\n";
    stream << "    std::filesystem::path stdout_log_path;\n";
    stream << "    std::filesystem::path stderr_log_path;\n";
    stream << "    bool capture_stdout = true;\n";
    stream << "    bool capture_stderr = true;\n";
    stream << "    int expected_exit_code = 0;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeProcessLaunch {\n";
    stream << "    std::filesystem::path executable_path;\n";
    stream << "    std::vector<std::string> arguments;\n";
    stream << "    std::filesystem::path working_directory;\n";
    stream << "    std::filesystem::path stdout_log_path;\n";
    stream << "    std::filesystem::path stderr_log_path;\n";
    stream << "    bool capture_stdout = true;\n";
    stream << "    bool capture_stderr = true;\n";
    stream << "    bool launch_attempted = false;\n";
    stream << "    bool launch_succeeded = false;\n";
    stream << "    int exit_code = 0;\n";
    stream << "    int expected_exit_code = 0;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeHostFailureEvaluation {\n";
    stream << "    bool launch_failed = true;\n";
    stream << "    bool should_use_fallback_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeMissingResponseEvaluation {\n";
    stream << "    bool response_missing = true;\n";
    stream << "    bool should_use_fallback_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeResponseValidationEvaluation {\n";
    stream << "    bool validation_failed = true;\n";
    stream << "    bool should_use_fallback_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgePayloadPlan {\n";
    stream << "    CopperfinRuntimeBridgeDispatchPlan dispatch_plan;\n";
    stream << "    std::string request_payload_shape;\n";
    stream << "    std::string response_payload_shape;\n";
    stream << "    std::vector<std::string> request_fields;\n";
    stream << "    std::vector<std::string> response_fields;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeInterpretationPlan {\n";
    stream << "    CopperfinRuntimeBridgePayloadPlan payload_plan;\n";
    stream << "    std::string status_field;\n";
    stream << "    std::string value_field;\n";
    stream << "    std::string diagnostics_field;\n";
    stream << "    std::string wrapper_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeFailurePolicyPlan {\n";
    stream << "    CopperfinRuntimeBridgeInterpretationPlan interpretation_plan;\n";
    stream << "    bool fail_on_nonzero_exit = true;\n";
    stream << "    bool fail_on_missing_response = true;\n";
    stream << "    std::string diagnostics_fallback;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeResponseValidationPlan {\n";
    stream << "    CopperfinRuntimeBridgeFailurePolicyPlan failure_policy_plan;\n";
    stream << "    std::string expected_response_media_type;\n";
    stream << "    std::string expected_schema_version;\n";
    stream << "    std::vector<std::string> required_response_fields;\n";
    stream << "    std::string success_status_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeRequestArtifact {\n";
    stream << "    CopperfinRuntimeBridgeResponseValidationPlan response_validation_plan;\n";
    stream << "    std::string request_document;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeRequestWritePlan {\n";
    stream << "    CopperfinRuntimeBridgeRequestArtifact request_artifact;\n";
    stream << "    std::filesystem::path target_path;\n";
    stream << "    std::string write_mode;\n";
    stream << "    bool ensure_parent_directory = true;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeResponseReadPlan {\n";
    stream << "    CopperfinRuntimeBridgeRequestWritePlan request_write_plan;\n";
    stream << "    std::filesystem::path source_path;\n";
    stream << "    std::string read_mode;\n";
    stream << "    bool require_existing_response = true;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeResponseArtifact {\n";
    stream << "    CopperfinRuntimeBridgeResponseReadPlan response_read_plan;\n";
    stream << "    std::string response_document;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeResponseParsePlan {\n";
    stream << "    CopperfinRuntimeBridgeResponseArtifact response_artifact;\n";
    stream << "    std::string parser_kind;\n";
    stream << "    std::string status_field;\n";
    stream << "    std::string value_field;\n";
    stream << "    std::string diagnostics_field;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeResponseParseAdmission {\n";
    stream << "    bool should_parse_response = false;\n";
    stream << "    bool should_use_fallback_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeParsedResponse {\n";
    stream << "    std::string status_value;\n";
    stream << "    std::string return_value_representation;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeInterpretedResultPlan {\n";
    stream << "    CopperfinRuntimeBridgeResponseParsePlan response_parse_plan;\n";
    stream << "    std::string success_status_value;\n";
    stream << "    std::string wrapper_return_surface;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeInterpretedResultAdmission {\n";
    stream << "    bool should_interpret_result = false;\n";
    stream << "    bool should_use_fallback_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeInterpretedResult {\n";
    stream << "    bool matched_success_status = false;\n";
    stream << "    std::string selected_return_value_representation;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string wrapper_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeNativeReturnPlan {\n";
    stream << "    CopperfinRuntimeBridgeInterpretedResultPlan interpreted_result_plan;\n";
    stream << "    std::string success_value_representation;\n";
    stream << "    int success_int_value = -1;\n";
    stream << "    std::string fallback_value_representation;\n";
    stream << "    int fallback_int_value = -1;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeNativeReturnAdmission {\n";
    stream << "    bool should_materialize_native_return = false;\n";
    stream << "    bool should_use_fallback_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeNativeReturn {\n";
    stream << "    bool matched_success_status = false;\n";
    stream << "    std::string selected_value_representation;\n";
    stream << "    int selected_int_value = -1;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeOutcomeSelectionPlan {\n";
    stream << "    CopperfinRuntimeBridgeNativeReturnPlan native_return_plan;\n";
    stream << "    std::string success_condition;\n";
    stream << "    std::string fallback_condition;\n";
    stream << "    std::string diagnostics_field;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeOutcomeSelectionAdmission {\n";
    stream << "    bool should_select_outcome = false;\n";
    stream << "    bool should_use_fallback_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeOutcomeSelection {\n";
    stream << "    bool matched_success_status = false;\n";
    stream << "    std::string selected_condition;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string selected_value_representation;\n";
    stream << "    int selected_int_value = -1;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturnMaterializationPlan {\n";
    stream << "    CopperfinRuntimeBridgeOutcomeSelectionPlan outcome_selection_plan;\n";
    stream << "    std::string success_return_statement;\n";
    stream << "    std::string fallback_return_statement;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturnMaterializationAdmission {\n";
    stream << "    bool should_materialize_return = false;\n";
    stream << "    bool should_use_fallback_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string fallback_return_value;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturnMaterialization {\n";
    stream << "    bool matched_success_status = false;\n";
    stream << "    std::string success_condition;\n";
    stream << "    std::string selected_condition;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string success_return_statement;\n";
    stream << "    std::string fallback_return_statement;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturnEmissionPlan {\n";
    stream << "    CopperfinRuntimeBridgeReturnMaterializationPlan return_materialization_plan;\n";
    stream << "    std::string success_branch_statement;\n";
    stream << "    std::string fallback_branch_statement;\n";
    stream << "    std::string emitted_return_block;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturnEmissionAdmission {\n";
    stream << "    bool should_emit_return = false;\n";
    stream << "    bool should_use_fallback_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string emitted_return_block;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturnEmission {\n";
    stream << "    bool matched_success_status = false;\n";
    stream << "    std::string selected_condition;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string success_branch_statement;\n";
    stream << "    std::string fallback_branch_statement;\n";
    stream << "    std::string emitted_return_block;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeFinalReturnAdoptionPlan {\n";
    stream << "    CopperfinRuntimeBridgeReturnEmissionPlan return_emission_plan;\n";
    stream << "    std::string placeholder_return_statement;\n";
    stream << "    std::string adopted_return_block;\n";
    stream << "    std::string adoption_mode;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeFinalReturnAdoptionAdmission {\n";
    stream << "    bool should_adopt_return = false;\n";
    stream << "    bool should_use_placeholder_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string selected_return_block;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeFinalReturnAdoption {\n";
    stream << "    bool matched_success_status = false;\n";
    stream << "    std::string selected_condition;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string placeholder_return_statement;\n";
    stream << "    std::string adopted_return_block;\n";
    stream << "    std::string adoption_mode;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturnActivationPlan {\n";
    stream << "    CopperfinRuntimeBridgeFinalReturnAdoptionPlan final_return_adoption_plan;\n";
    stream << "    bool activates_adopted_return = false;\n";
    stream << "    std::string activation_mode;\n";
    stream << "    std::string active_return_block;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturnActivationAdmission {\n";
    stream << "    bool should_activate_return = false;\n";
    stream << "    bool should_emit_placeholder_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string selected_return_block;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeReturnActivation {\n";
    stream << "    bool matched_success_status = false;\n";
    stream << "    std::string selected_condition;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    bool activates_adopted_return = false;\n";
    stream << "    std::string activation_mode;\n";
    stream << "    std::string active_return_block;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeStubReturnPlan {\n";
    stream << "    CopperfinRuntimeBridgeReturnActivationPlan return_activation_plan;\n";
    stream << "    std::string emitted_return_statement;\n";
    stream << "    std::string deferred_return_block;\n";
    stream << "    bool emits_placeholder_return = true;\n";
    stream << "    std::string activation_mode;\n";
    stream << "    std::string adoption_mode;\n";
    stream << "    bool keeps_placeholder_return_active = true;\n";
    stream << "    bool adopts_placeholder_replacement = true;\n";
    stream << "    int placeholder_fallback_int_value = -1;\n";
    stream << "    std::string placeholder_fallback_value_representation;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeStubReturnAdmission {\n";
    stream << "    bool should_route_stub_return = false;\n";
    stream << "    bool should_emit_placeholder_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string selected_return_statement;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeStubReturn {\n";
    stream << "    bool matched_success_status = false;\n";
    stream << "    std::string selected_condition;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string emitted_return_statement;\n";
    stream << "    std::string deferred_return_block;\n";
    stream << "    bool emits_placeholder_return = true;\n";
    stream << "    std::string activation_mode;\n";
    stream << "    std::string adoption_mode;\n";
    stream << "    bool keeps_placeholder_return_active = true;\n";
    stream << "    bool adopts_placeholder_replacement = true;\n";
    stream << "    int placeholder_fallback_int_value = -1;\n";
    stream << "    std::string placeholder_fallback_value_representation;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "using CopperfinRuntimeBridgeIntReturnAdapter = int (*)(int);\n\n";
    stream << "struct CopperfinRuntimeBridgeStubEmissionWrapper {\n";
    stream << "    std::string native_return_surface;\n";
    stream << "    CopperfinRuntimeBridgeIntReturnAdapter return_adapter = nullptr;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgePlaceholderReturnValuePlan {\n";
    stream << "    CopperfinRuntimeBridgeStubReturnPlan stub_return_plan;\n";
    stream << "    bool emits_placeholder_return = true;\n";
    stream << "    std::string emitted_return_statement;\n";
    stream << "    std::string deferred_return_block;\n";
    stream << "    std::string activation_mode;\n";
    stream << "    std::string adoption_mode;\n";
    stream << "    bool keeps_placeholder_return_active = true;\n";
    stream << "    bool adopts_placeholder_replacement = true;\n";
    stream << "    int fallback_int_value = -1;\n";
    stream << "    std::string fallback_value_representation;\n";
    stream << "    CopperfinRuntimeBridgeStubEmissionWrapper stub_emission_wrapper;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgePlaceholderReturnValueAdmission {\n";
    stream << "    bool should_emit_placeholder_return = true;\n";
    stream << "    bool should_keep_deferred_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    std::string selected_return_statement;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgePlaceholderReturnValue {\n";
    stream << "    bool matched_success_status = false;\n";
    stream << "    std::string selected_condition;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    bool emits_placeholder_return = true;\n";
    stream << "    std::string emitted_return_statement;\n";
    stream << "    std::string deferred_return_block;\n";
    stream << "    std::string activation_mode;\n";
    stream << "    std::string adoption_mode;\n";
    stream << "    bool keeps_placeholder_return_active = true;\n";
    stream << "    bool adopts_placeholder_replacement = true;\n";
    stream << "    int fallback_int_value = -1;\n";
    stream << "    std::string fallback_value_representation;\n";
    stream << "    std::string native_return_surface;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgePlaceholderReturnIntAdmission {\n";
    stream << "    bool should_return_int = true;\n";
    stream << "    bool should_emit_placeholder_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    int selected_int_value = -1;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeStubEmissionAdmission {\n";
    stream << "    bool should_emit_stub_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    int selected_int_value = -1;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeStubEmission {\n";
    stream << "    bool should_emit_stub_return = true;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    int emitted_int_value = -1;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeStubEmissionReturnSurface {\n";
    stream << "    std::string native_return_surface;\n";
    stream << "    std::string diagnostics_value;\n";
    stream << "    int emitted_int_value = -1;\n";
    stream << "};\n\n";
    stream << "static CopperfinRuntimeBridgeDescriptor copperfin_build_runtime_bridge_descriptor(\n";
    stream << "    const char* export_name,\n";
    stream << "    const char* routine_kind,\n";
    stream << "    const char* source_path,\n";
    stream << "    unsigned int source_line,\n";
    stream << "    const char* parameter_declaration_kind,\n";
    stream << "    const char* parameter_names,\n";
    stream << "    unsigned int parameter_count,\n";
    stream << "    void* symbol_address) {\n";
    stream << "    return CopperfinRuntimeBridgeDescriptor{\n";
    stream << "        export_name,\n";
    stream << "        routine_kind,\n";
    stream << "        source_path,\n";
    stream << "        source_line,\n";
    stream << "        parameter_declaration_kind,\n";
    stream << "        parameter_names,\n";
    stream << "        parameter_count,\n";
    stream << "        copperfin_runtime_manifest_path(symbol_address),\n";
    stream << "        copperfin_runtime_host_path(symbol_address)};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_manifest_flag() {\n";
    stream << "    return \"--manifest\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_library_export_flag() {\n";
    stream << "    return \"--library-export\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_routine_kind_flag() {\n";
    stream << "    return \"--routine-kind\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_source_path_flag() {\n";
    stream << "    return \"--source-path\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_source_line_flag() {\n";
    stream << "    return \"--source-line\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_parameter_declaration_flag() {\n";
    stream << "    return \"--parameter-declaration\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_parameter_names_flag() {\n";
    stream << "    return \"--parameter-names\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_parameter_count_flag() {\n";
    stream << "    return \"--parameter-count\";\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeInvocation copperfin_build_runtime_bridge_invocation(\n";
    stream << "    const CopperfinRuntimeBridgeDescriptor& descriptor) {\n";
    stream << "    return CopperfinRuntimeBridgeInvocation{\n";
    stream << "        descriptor,\n";
    stream << "        {\n";
    stream << "            descriptor.runtime_host_path.string(),\n";
    stream << "            copperfin_runtime_bridge_manifest_flag(),\n";
    stream << "            descriptor.manifest_path.string(),\n";
    stream << "            copperfin_runtime_bridge_library_export_flag(),\n";
    stream << "            descriptor.export_name,\n";
    stream << "            copperfin_runtime_bridge_routine_kind_flag(),\n";
    stream << "            descriptor.routine_kind,\n";
    stream << "            copperfin_runtime_bridge_source_path_flag(),\n";
    stream << "            descriptor.source_path,\n";
    stream << "            copperfin_runtime_bridge_source_line_flag(),\n";
    stream << "            std::to_string(descriptor.source_line),\n";
    stream << "            copperfin_runtime_bridge_parameter_declaration_flag(),\n";
    stream << "            descriptor.parameter_declaration_kind,\n";
    stream << "            copperfin_runtime_bridge_parameter_names_flag(),\n";
    stream << "            descriptor.parameter_names,\n";
    stream << "            copperfin_runtime_bridge_parameter_count_flag(),\n";
    stream << "            std::to_string(descriptor.parameter_count)}};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeCall copperfin_build_runtime_bridge_call(\n";
    stream << "    const CopperfinRuntimeBridgeInvocation& invocation,\n";
    stream << "    std::vector<CopperfinRuntimeBridgeParameter> parameters) {\n";
    stream << "    return CopperfinRuntimeBridgeCall{invocation, std::move(parameters)};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_escape_runtime_bridge_json_string(const std::string& value) {\n";
    stream << "    std::string escaped;\n";
    stream << "    escaped.reserve(value.size());\n";
    stream << "    for (const char ch : value) {\n";
    stream << "        switch (ch) {\n";
    stream << "        case '\\\\':\n";
    stream << "            escaped += \"\\\\\\\\\";\n";
    stream << "            break;\n";
    stream << "        case '\"':\n";
    stream << "            escaped += \"\\\\\\\"\";\n";
    stream << "            break;\n";
    stream << "        case '\\n':\n";
    stream << "            escaped += \"\\\\n\";\n";
    stream << "            break;\n";
    stream << "        case '\\r':\n";
    stream << "            escaped += \"\\\\r\";\n";
    stream << "            break;\n";
    stream << "        case '\\t':\n";
    stream << "            escaped += \"\\\\t\";\n";
    stream << "            break;\n";
    stream << "        default:\n";
    stream << "            escaped.push_back(ch);\n";
    stream << "            break;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    return escaped;\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_payload_shape_field_name() {\n";
    stream << "    return \"payload_shape\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_export_name_field_name() {\n";
    stream << "    return \"export_name\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_parameter_count_field_name() {\n";
    stream << "    return \"parameter_count\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_parameters_field_name() {\n";
    stream << "    return \"parameters\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_request_media_type_field_name() {\n";
    stream << "    return \"request_media_type\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_parameter_name_field_name() {\n";
    stream << "    return \"name\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_parameter_value_field_name() {\n";
    stream << "    return \"value\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_parameter_surface_field_name() {\n";
    stream << "    return \"surface\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_request_document(\n";
    stream << "    const CopperfinRuntimeBridgeCall& call,\n";
    stream << "    const CopperfinRuntimeBridgePayloadPlan& payload_plan,\n";
    stream << "    const std::string& request_media_type) {\n";
    stream << "    std::ostringstream request_stream;\n";
    stream << "    request_stream << \"{\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_payload_shape_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(payload_plan.request_payload_shape)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_export_name_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.export_name)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_parameter_count_field_name()\n";
    stream << "                   << \"\\\": \" << call.invocation.descriptor.parameter_count << \",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_request_media_type_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(request_media_type)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_parameters_field_name()\n";
    stream << "                   << \"\\\": [\\n\";\n";
    stream << "    for (std::size_t index = 0; index < call.parameters.size(); ++index) {\n";
    stream << "        const auto& parameter = call.parameters[index];\n";
    stream << "        request_stream << \"    {\\\"\"\n";
    stream << "                       << copperfin_build_runtime_bridge_parameter_name_field_name()\n";
    stream << "                       << \"\\\": \\\"\"\n";
    stream << "                       << copperfin_escape_runtime_bridge_json_string(parameter.parameter_name)\n";
    stream << "                       << \"\\\", \\\"\"\n";
    stream << "                       << copperfin_build_runtime_bridge_parameter_value_field_name()\n";
    stream << "                       << \"\\\": \\\"\"\n";
    stream << "                       << copperfin_escape_runtime_bridge_json_string(parameter.value_representation)\n";
    stream << "                       << \"\\\", \\\"\"\n";
    stream << "                       << copperfin_build_runtime_bridge_parameter_surface_field_name()\n";
    stream << "                       << \"\\\": \\\"\"\n";
    stream << "                       << copperfin_escape_runtime_bridge_json_string(parameter.call_surface)\n";
    stream << "                       << \"\\\"}\";\n";
    stream << "        if (index + 1U < call.parameters.size()) {\n";
    stream << "            request_stream << \",\";\n";
    stream << "        }\n";
    stream << "        request_stream << \"\\n\";\n";
    stream << "    }\n";
    stream << "    request_stream << \"  ]\\n\";\n";
    stream << "    request_stream << \"}\";\n";
    stream << "    return request_stream.str();\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeResult copperfin_build_runtime_bridge_result(\n";
    stream << "    CopperfinRuntimeBridgeCall call,\n";
    stream << "    CopperfinRuntimeBridgeReturn return_binding) {\n";
    stream << "    return CopperfinRuntimeBridgeResult{std::move(call), std::move(return_binding)};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturn copperfin_build_runtime_bridge_placeholder_return_binding(\n";
    stream << "    std::string return_surface) {\n";
    stream << "    return CopperfinRuntimeBridgeReturn{std::to_string(-1), std::move(return_surface)};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_library_export_env_var() {\n";
    stream << "    return \"COPPERFIN_LIBRARY_EXPORT\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_routine_kind_env_var() {\n";
    stream << "    return \"COPPERFIN_ROUTINE_KIND\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_source_path_env_var() {\n";
    stream << "    return \"COPPERFIN_SOURCE_PATH\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_parameter_count_env_var() {\n";
    stream << "    return \"COPPERFIN_PARAMETER_COUNT\";\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeLaunchPlan copperfin_build_runtime_bridge_launch_plan(\n";
    stream << "    CopperfinRuntimeBridgeResult result) {\n";
    stream << "    return CopperfinRuntimeBridgeLaunchPlan{\n";
    stream << "        result,\n";
    stream << "        result.call.invocation.descriptor.runtime_host_path.parent_path(),\n";
    stream << "        {\n";
    stream << "            {copperfin_runtime_bridge_library_export_env_var(), result.call.invocation.descriptor.export_name},\n";
    stream << "            {copperfin_runtime_bridge_routine_kind_env_var(), result.call.invocation.descriptor.routine_kind},\n";
    stream << "            {copperfin_runtime_bridge_source_path_env_var(), result.call.invocation.descriptor.source_path},\n";
    stream << "            {copperfin_runtime_bridge_parameter_count_env_var(), std::to_string(result.call.invocation.descriptor.parameter_count)}}};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_stdout_log_suffix() {\n";
    stream << "    return \".stdout.log\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_stderr_log_suffix() {\n";
    stream << "    return \".stderr.log\";\n";
    stream << "}\n\n";
    stream << "static int copperfin_runtime_bridge_expected_exit_code() {\n";
    stream << "    return 0;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeObservationPlan copperfin_build_runtime_bridge_observation_plan(\n";
    stream << "    CopperfinRuntimeBridgeLaunchPlan launch_plan) {\n";
    stream << "    const auto export_name = launch_plan.result.call.invocation.descriptor.export_name;\n";
    stream << "    const auto base_directory = launch_plan.working_directory;\n";
    stream << "    return CopperfinRuntimeBridgeObservationPlan{\n";
    stream << "        std::move(launch_plan),\n";
    stream << "        base_directory / (std::string(export_name) + copperfin_runtime_bridge_stdout_log_suffix()),\n";
    stream << "        base_directory / (std::string(export_name) + copperfin_runtime_bridge_stderr_log_suffix()),\n";
    stream << "        copperfin_runtime_bridge_expected_exit_code()};\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_capture_stdout_policy() {\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_capture_stderr_policy() {\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeExecutionPlan copperfin_build_runtime_bridge_execution_plan(\n";
    stream << "    CopperfinRuntimeBridgeObservationPlan observation_plan) {\n";
    stream << "    return CopperfinRuntimeBridgeExecutionPlan{\n";
    stream << "        observation_plan,\n";
    stream << "        observation_plan.launch_plan.result.call.invocation.descriptor.runtime_host_path,\n";
    stream << "        observation_plan.launch_plan.result.call.invocation.arguments,\n";
    stream << "        copperfin_runtime_bridge_capture_stdout_policy(),\n";
    stream << "        copperfin_runtime_bridge_capture_stderr_policy()};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_request_artifact_suffix() {\n";
    stream << "    return \".request.json\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_response_artifact_suffix() {\n";
    stream << "    return \".response.json\";\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeTransportPlan copperfin_build_runtime_bridge_transport_plan(\n";
    stream << "    CopperfinRuntimeBridgeExecutionPlan execution_plan) {\n";
    stream << "    const auto export_name =\n";
    stream << "        execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.export_name;\n";
    stream << "    const auto base_directory =\n";
    stream << "        execution_plan.observation_plan.launch_plan.working_directory;\n";
    stream << "    return CopperfinRuntimeBridgeTransportPlan{\n";
    stream << "        std::move(execution_plan),\n";
    stream << "        base_directory / (std::string(export_name) + copperfin_runtime_bridge_request_artifact_suffix()),\n";
    stream << "        base_directory / (std::string(export_name) + copperfin_runtime_bridge_response_artifact_suffix())};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_request_media_type_value() {\n";
    stream << "    return \"application/vnd.copperfin.runtime-bridge-request+json\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_media_type_value() {\n";
    stream << "    return \"application/vnd.copperfin.runtime-bridge-response+json\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_schema_version_value() {\n";
    stream << "    return \"v1\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_request_path_argument_name() {\n";
    stream << "    return \"--request-path\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_path_argument_name() {\n";
    stream << "    return \"--response-path\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_request_media_type_argument_name() {\n";
    stream << "    return \"--request-media-type\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_media_type_argument_name() {\n";
    stream << "    return \"--response-media-type\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_schema_version_argument_name() {\n";
    stream << "    return \"--schema-version\";\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeSerializationPlan copperfin_build_runtime_bridge_serialization_plan(\n";
    stream << "    CopperfinRuntimeBridgeTransportPlan transport_plan) {\n";
    stream << "    return CopperfinRuntimeBridgeSerializationPlan{\n";
    stream << "        std::move(transport_plan),\n";
    stream << "        copperfin_build_runtime_bridge_request_media_type_value(),\n";
    stream << "        copperfin_build_runtime_bridge_response_media_type_value(),\n";
    stream << "        copperfin_build_runtime_bridge_schema_version_value()};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeDispatchPlan copperfin_build_runtime_bridge_dispatch_plan(\n";
    stream << "    CopperfinRuntimeBridgeSerializationPlan serialization_plan) {\n";
    stream << "    auto arguments = serialization_plan.transport_plan.execution_plan.arguments;\n";
    stream << "    arguments.push_back(copperfin_build_runtime_bridge_request_path_argument_name());\n";
    stream << "    arguments.push_back(serialization_plan.transport_plan.request_path.string());\n";
    stream << "    arguments.push_back(copperfin_build_runtime_bridge_response_path_argument_name());\n";
    stream << "    arguments.push_back(serialization_plan.transport_plan.response_path.string());\n";
    stream << "    arguments.push_back(copperfin_build_runtime_bridge_request_media_type_argument_name());\n";
    stream << "    arguments.push_back(serialization_plan.request_media_type);\n";
    stream << "    arguments.push_back(copperfin_build_runtime_bridge_response_media_type_argument_name());\n";
    stream << "    arguments.push_back(serialization_plan.response_media_type);\n";
    stream << "    arguments.push_back(copperfin_build_runtime_bridge_schema_version_argument_name());\n";
    stream << "    arguments.push_back(serialization_plan.schema_version);\n";
    stream << "    return CopperfinRuntimeBridgeDispatchPlan{std::move(serialization_plan), std::move(arguments)};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeDispatchExecution copperfin_runtime_bridge_execute_dispatch(\n";
    stream << "    const CopperfinRuntimeBridgeDispatchPlan& plan) {\n";
    stream << "    const auto& execution_plan = plan.serialization_plan.transport_plan.execution_plan;\n";
    stream << "    const auto& observation_plan = execution_plan.observation_plan;\n";
    stream << "    const auto& launch_plan = observation_plan.launch_plan;\n";
    stream << "    return CopperfinRuntimeBridgeDispatchExecution{\n";
    stream << "        execution_plan.executable_path,\n";
    stream << "        plan.arguments,\n";
    stream << "        launch_plan.working_directory,\n";
    stream << "        observation_plan.stdout_log_path,\n";
    stream << "        observation_plan.stderr_log_path,\n";
    stream << "        execution_plan.capture_stdout,\n";
    stream << "        execution_plan.capture_stderr,\n";
    stream << "        observation_plan.expected_exit_code};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_launch_process(\n";
    stream << "    const CopperfinRuntimeBridgeDispatchExecution& dispatch_execution) {\n";
    stream << "    return CopperfinRuntimeBridgeProcessLaunch{\n";
    stream << "        dispatch_execution.executable_path,\n";
    stream << "        dispatch_execution.arguments,\n";
    stream << "        dispatch_execution.working_directory,\n";
    stream << "        dispatch_execution.stdout_log_path,\n";
    stream << "        dispatch_execution.stderr_log_path,\n";
    stream << "        dispatch_execution.capture_stdout,\n";
    stream << "        dispatch_execution.capture_stderr,\n";
    stream << "        false,\n";
    stream << "        false,\n";
    stream << "        dispatch_execution.expected_exit_code,\n";
    stream << "        dispatch_execution.expected_exit_code};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeHostFailureEvaluation copperfin_runtime_bridge_evaluate_host_failure(\n";
    stream << "    const CopperfinRuntimeBridgeProcessLaunch& process_launch,\n";
    stream << "    const CopperfinRuntimeBridgeFailurePolicyPlan& failure_policy_plan) {\n";
    stream << "    const bool launch_failed = !process_launch.launch_succeeded;\n";
    stream << "    const bool should_use_fallback_return =\n";
    stream << "        launch_failed && failure_policy_plan.fail_on_nonzero_exit;\n";
    stream << "    return CopperfinRuntimeBridgeHostFailureEvaluation{\n";
    stream << "        launch_failed,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        should_use_fallback_return ? failure_policy_plan.diagnostics_fallback : std::string{},\n";
    stream << "        failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeMissingResponseEvaluation copperfin_runtime_bridge_evaluate_missing_response(\n";
    stream << "    const CopperfinRuntimeBridgeHostFailureEvaluation& host_failure,\n";
    stream << "    const CopperfinRuntimeBridgeResponseReadPlan& response_read_plan) {\n";
    stream << "    const auto& failure_policy_plan =\n";
    stream << "        response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan;\n";
    stream << "    const bool response_missing = response_read_plan.require_existing_response;\n";
    stream << "    const bool should_use_fallback_return =\n";
    stream << "        host_failure.should_use_fallback_return || response_missing;\n";
    stream << "    return CopperfinRuntimeBridgeMissingResponseEvaluation{\n";
    stream << "        response_missing,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        should_use_fallback_return ? failure_policy_plan.diagnostics_fallback : host_failure.diagnostics_value,\n";
    stream << "        failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeResponseValidationEvaluation copperfin_runtime_bridge_evaluate_response_validation(\n";
    stream << "    const CopperfinRuntimeBridgeMissingResponseEvaluation& missing_response,\n";
    stream << "    const CopperfinRuntimeBridgeResponseValidationPlan& response_validation_plan) {\n";
    stream << "    const bool validation_failed = missing_response.should_use_fallback_return;\n";
    stream << "    const bool should_use_fallback_return = validation_failed;\n";
    stream << "    return CopperfinRuntimeBridgeResponseValidationEvaluation{\n";
    stream << "        validation_failed,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        should_use_fallback_return ? response_validation_plan.failure_policy_plan.diagnostics_fallback : std::string{},\n";
    stream << "        response_validation_plan.failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_status_field_name() {\n";
    stream << "    return \"status\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_return_value_field_name() {\n";
    stream << "    return \"return_value\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_diagnostics_field_name() {\n";
    stream << "    return \"diagnostics\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_request_payload_shape_name() {\n";
    stream << "    return \"bridge_request_v1\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_payload_shape_name() {\n";
    stream << "    return \"bridge_response_v1\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_media_type_field_name() {\n";
    stream << "    return \"response_media_type\";\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgePayloadPlan copperfin_build_runtime_bridge_payload_plan(\n";
    stream << "    CopperfinRuntimeBridgeDispatchPlan dispatch_plan) {\n";
    stream << "    return CopperfinRuntimeBridgePayloadPlan{\n";
    stream << "        std::move(dispatch_plan),\n";
    stream << "        copperfin_build_runtime_bridge_request_payload_shape_name(),\n";
    stream << "        copperfin_build_runtime_bridge_response_payload_shape_name(),\n";
    stream << "        {copperfin_build_runtime_bridge_export_name_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_parameter_count_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_parameters_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_request_media_type_field_name()},\n";
    stream << "        {copperfin_build_runtime_bridge_status_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_return_value_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_response_media_type_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_diagnostics_field_name()}};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeInterpretationPlan copperfin_build_runtime_bridge_interpretation_plan(\n";
    stream << "    CopperfinRuntimeBridgePayloadPlan payload_plan,\n";
    stream << "    std::string wrapper_return_surface) {\n";
    stream << "    return CopperfinRuntimeBridgeInterpretationPlan{\n";
    stream << "        std::move(payload_plan),\n";
    stream << "        copperfin_build_runtime_bridge_status_field_name(),\n";
    stream << "        copperfin_build_runtime_bridge_return_value_field_name(),\n";
    stream << "        copperfin_build_runtime_bridge_diagnostics_field_name(),\n";
    stream << "        std::move(wrapper_return_surface)};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_failure_diagnostics_value() {\n";
    stream << "    return \"runtime_host_failure\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_success_status_value() {\n";
    stream << "    return \"ok\";\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_fail_on_nonzero_exit_policy() {\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_fail_on_missing_response_policy() {\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeFailurePolicyPlan copperfin_build_runtime_bridge_failure_policy_plan(\n";
    stream << "    CopperfinRuntimeBridgeInterpretationPlan interpretation_plan,\n";
    stream << "    std::string fallback_return_value) {\n";
    stream << "    return CopperfinRuntimeBridgeFailurePolicyPlan{\n";
    stream << "        std::move(interpretation_plan),\n";
    stream << "        copperfin_runtime_bridge_fail_on_nonzero_exit_policy(),\n";
    stream << "        copperfin_runtime_bridge_fail_on_missing_response_policy(),\n";
    stream << "        copperfin_build_runtime_bridge_failure_diagnostics_value(),\n";
    stream << "        std::move(fallback_return_value)};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeResponseValidationPlan copperfin_build_runtime_bridge_response_validation_plan(\n";
    stream << "    CopperfinRuntimeBridgeFailurePolicyPlan failure_policy_plan) {\n";
    stream << "    const auto expected_response_media_type =\n";
    stream << "        failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.response_media_type;\n";
    stream << "    const auto expected_schema_version =\n";
    stream << "        failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.schema_version;\n";
    stream << "    const auto required_response_fields =\n";
    stream << "        failure_policy_plan.interpretation_plan.payload_plan.response_fields;\n";
    stream << "    return CopperfinRuntimeBridgeResponseValidationPlan{\n";
    stream << "        std::move(failure_policy_plan),\n";
    stream << "        expected_response_media_type,\n";
    stream << "        expected_schema_version,\n";
    stream << "        required_response_fields,\n";
    stream << "        copperfin_build_runtime_bridge_success_status_value()};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeRequestArtifact copperfin_build_runtime_bridge_request_artifact(\n";
    stream << "    CopperfinRuntimeBridgeResponseValidationPlan response_validation_plan) {\n";
    stream << "    const auto request_document = copperfin_build_runtime_bridge_request_document(\n";
    stream << "        response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call,\n";
    stream << "        response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan,\n";
    stream << "        response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.request_media_type);\n";
    stream << "    return CopperfinRuntimeBridgeRequestArtifact{\n";
    stream << "        std::move(response_validation_plan),\n";
    stream << "        std::move(request_document)};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_request_write_mode() {\n";
    stream << "    return \"overwrite\";\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_ensure_parent_directory_policy() {\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeRequestWritePlan copperfin_build_runtime_bridge_request_write_plan(\n";
    stream << "    CopperfinRuntimeBridgeRequestArtifact request_artifact) {\n";
    stream << "    const auto target_path =\n";
    stream << "        request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.request_path;\n";
    stream << "    const auto write_mode = copperfin_build_runtime_bridge_request_write_mode();\n";
    stream << "    return CopperfinRuntimeBridgeRequestWritePlan{\n";
    stream << "        std::move(request_artifact),\n";
    stream << "        target_path,\n";
    stream << "        write_mode,\n";
    stream << "        copperfin_runtime_bridge_ensure_parent_directory_policy()};\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_execute_write_request(\n";
    stream << "    const CopperfinRuntimeBridgeRequestWritePlan& plan) {\n";
    stream << "    if (plan.ensure_parent_directory) {\n";
    stream << "        std::filesystem::create_directories(plan.target_path.parent_path());\n";
    stream << "    }\n";
    stream << "    std::ofstream out(plan.target_path);\n";
    stream << "    if (!out) {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    out << plan.request_artifact.request_document;\n";
    stream << "    return out.good();\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_read_mode() {\n";
    stream << "    return \"read_text\";\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_require_existing_response_policy() {\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeResponseReadPlan copperfin_build_runtime_bridge_response_read_plan(\n";
    stream << "    CopperfinRuntimeBridgeRequestWritePlan request_write_plan) {\n";
    stream << "    const auto source_path =\n";
    stream << "        request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.response_path;\n";
    stream << "    const auto read_mode = copperfin_build_runtime_bridge_response_read_mode();\n";
    stream << "    return CopperfinRuntimeBridgeResponseReadPlan{\n";
    stream << "        std::move(request_write_plan),\n";
    stream << "        source_path,\n";
    stream << "        read_mode,\n";
    stream << "        copperfin_runtime_bridge_require_existing_response_policy()};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_empty_response_document() {\n";
    stream << "    return \"\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_execute_read_response(\n";
    stream << "    const CopperfinRuntimeBridgeResponseReadPlan& plan) {\n";
    stream << "    if (plan.require_existing_response && !std::filesystem::exists(plan.source_path)) {\n";
    stream << "        return copperfin_build_runtime_bridge_empty_response_document();\n";
    stream << "    }\n";
    stream << "    std::ifstream input(plan.source_path, std::ios::binary);\n";
    stream << "    if (!input) {\n";
    stream << "        return copperfin_build_runtime_bridge_empty_response_document();\n";
    stream << "    }\n";
    stream << "    std::ostringstream response_document;\n";
    stream << "    response_document << input.rdbuf();\n";
    stream << "    return response_document.str();\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeResponseArtifact copperfin_build_runtime_bridge_response_artifact(\n";
    stream << "    CopperfinRuntimeBridgeResponseReadPlan response_read_plan) {\n";
    stream << "    const auto response_document = copperfin_build_runtime_bridge_empty_response_document();\n";
    stream << "    return CopperfinRuntimeBridgeResponseArtifact{\n";
    stream << "        std::move(response_read_plan),\n";
    stream << "        response_document};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_parse_kind() {\n";
    stream << "    return \"json_field_map\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_extract_json_field(\n";
    stream << "    const std::string& response_document,\n";
    stream << "    const std::string& field_name) {\n";
    stream << "    const auto field_token = std::string(\"\\\"\") + field_name + \"\\\"\";\n";
    stream << "    const auto field_offset = response_document.find(field_token);\n";
    stream << "    if (field_offset == std::string::npos) {\n";
    stream << "        return \"\";\n";
    stream << "    }\n";
    stream << "    const auto colon_offset = response_document.find(':', field_offset + field_token.size());\n";
    stream << "    if (colon_offset == std::string::npos) {\n";
    stream << "        return \"\";\n";
    stream << "    }\n";
    stream << "    const auto value_start = response_document.find_first_not_of(\" \\t\\r\\n\", colon_offset + 1);\n";
    stream << "    if (value_start == std::string::npos) {\n";
    stream << "        return \"\";\n";
    stream << "    }\n";
    stream << "    if (response_document[value_start] == '\"') {\n";
    stream << "        const auto string_end = response_document.find('\"', value_start + 1);\n";
    stream << "        if (string_end == std::string::npos) {\n";
    stream << "            return \"\";\n";
    stream << "        }\n";
    stream << "        return response_document.substr(value_start + 1, string_end - value_start - 1);\n";
    stream << "    }\n";
    stream << "    const auto value_end = response_document.find_first_of(\",}\", value_start);\n";
    stream << "    return response_document.substr(\n";
    stream << "        value_start,\n";
    stream << "        value_end == std::string::npos ? std::string::npos : value_end - value_start);\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeParsedResponse copperfin_runtime_bridge_execute_parse_response(\n";
    stream << "    const CopperfinRuntimeBridgeResponseParsePlan& plan) {\n";
    stream << "    const auto& response_document = plan.response_artifact.response_document;\n";
    stream << "    return CopperfinRuntimeBridgeParsedResponse{\n";
    stream << "        copperfin_runtime_bridge_extract_json_field(response_document, plan.status_field),\n";
    stream << "        copperfin_runtime_bridge_extract_json_field(response_document, plan.value_field),\n";
    stream << "        copperfin_runtime_bridge_extract_json_field(response_document, plan.diagnostics_field)};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeResponseParsePlan copperfin_build_runtime_bridge_response_parse_plan(\n";
    stream << "    CopperfinRuntimeBridgeResponseArtifact response_artifact) {\n";
    stream << "    const auto& interpretation_plan =\n";
    stream << "        response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan;\n";
    stream << "    const auto parser_kind = copperfin_build_runtime_bridge_response_parse_kind();\n";
    stream << "    return CopperfinRuntimeBridgeResponseParsePlan{\n";
    stream << "        std::move(response_artifact),\n";
    stream << "        parser_kind,\n";
    stream << "        interpretation_plan.status_field,\n";
    stream << "        interpretation_plan.value_field,\n";
    stream << "        interpretation_plan.diagnostics_field};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeResponseParseAdmission copperfin_runtime_bridge_admit_response_parse(\n";
    stream << "    const CopperfinRuntimeBridgeResponseValidationEvaluation& response_validation_evaluation,\n";
    stream << "    const CopperfinRuntimeBridgeResponseParsePlan& response_parse_plan) {\n";
    stream << "    const auto& failure_policy_plan =\n";
    stream << "        response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan;\n";
    stream << "    const bool should_parse_response = !response_validation_evaluation.should_use_fallback_return;\n";
    stream << "    const bool should_use_fallback_return = !should_parse_response;\n";
    stream << "    return CopperfinRuntimeBridgeResponseParseAdmission{\n";
    stream << "        should_parse_response,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        should_use_fallback_return ? failure_policy_plan.diagnostics_fallback : response_validation_evaluation.diagnostics_value,\n";
    stream << "        failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeInterpretedResultPlan copperfin_build_runtime_bridge_interpreted_result_plan(\n";
    stream << "    CopperfinRuntimeBridgeResponseParsePlan response_parse_plan) {\n";
    stream << "    const auto& response_validation_plan =\n";
    stream << "        response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan;\n";
    stream << "    const auto& failure_policy_plan = response_validation_plan.failure_policy_plan;\n";
    stream << "    return CopperfinRuntimeBridgeInterpretedResultPlan{\n";
    stream << "        std::move(response_parse_plan),\n";
    stream << "        response_validation_plan.success_status_value,\n";
    stream << "        failure_policy_plan.interpretation_plan.wrapper_return_surface,\n";
    stream << "        failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeInterpretedResultAdmission copperfin_runtime_bridge_admit_interpreted_result(\n";
    stream << "    const CopperfinRuntimeBridgeResponseParseAdmission& response_parse_admission,\n";
    stream << "    const CopperfinRuntimeBridgeInterpretedResultPlan& interpreted_result_plan) {\n";
    stream << "    const auto& failure_policy_plan = interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan;\n";
    stream << "    const bool should_interpret_result = !response_parse_admission.should_use_fallback_return;\n";
    stream << "    const bool should_use_fallback_return = !should_interpret_result;\n";
    stream << "    return CopperfinRuntimeBridgeInterpretedResultAdmission{\n";
    stream << "        should_interpret_result,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        should_use_fallback_return ? failure_policy_plan.diagnostics_fallback : response_parse_admission.diagnostics_value,\n";
    stream << "        failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeInterpretedResult copperfin_runtime_bridge_execute_interpreted_result(\n";
    stream << "    const CopperfinRuntimeBridgeInterpretedResultPlan& plan) {\n";
    stream << "    const auto parsed_response = copperfin_runtime_bridge_execute_parse_response(plan.response_parse_plan);\n";
    stream << "    const bool matched_success_status = parsed_response.status_value == plan.success_status_value;\n";
    stream << "    const auto selected_return_value_representation = matched_success_status\n";
    stream << "        ? parsed_response.return_value_representation\n";
    stream << "        : plan.fallback_return_value;\n";
    stream << "    return CopperfinRuntimeBridgeInterpretedResult{\n";
    stream << "        matched_success_status,\n";
    stream << "        std::move(selected_return_value_representation),\n";
    stream << "        std::move(parsed_response.diagnostics_value),\n";
    stream << "        plan.wrapper_return_surface};\n";
    stream << "}\n\n";
    stream << "static int copperfin_runtime_bridge_default_int_value() {\n";
    stream << "    return -1;\n";
    stream << "}\n\n";
    stream << "static int copperfin_parse_runtime_bridge_int_value_representation(\n";
    stream << "    const std::string& value_representation) {\n";
    stream << "    std::istringstream value_stream(value_representation);\n";
    stream << "    int parsed_value = copperfin_runtime_bridge_default_int_value();\n";
    stream << "    value_stream >> parsed_value;\n";
    stream << "    return parsed_value;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeNativeReturnPlan copperfin_build_runtime_bridge_native_return_plan(\n";
    stream << "    const CopperfinRuntimeBridgeResult& result,\n";
    stream << "    CopperfinRuntimeBridgeInterpretedResultPlan interpreted_result_plan) {\n";
    stream << "    const int success_int_value = copperfin_parse_runtime_bridge_int_value_representation(\n";
    stream << "        result.return_binding.value_representation);\n";
    stream << "    const auto fallback_value_representation = interpreted_result_plan.fallback_return_value;\n";
    stream << "    const int fallback_int_value = copperfin_parse_runtime_bridge_int_value_representation(\n";
    stream << "        fallback_value_representation);\n";
    stream << "    return CopperfinRuntimeBridgeNativeReturnPlan{\n";
    stream << "        std::move(interpreted_result_plan),\n";
    stream << "        result.return_binding.value_representation,\n";
    stream << "        success_int_value,\n";
    stream << "        fallback_value_representation,\n";
    stream << "        fallback_int_value,\n";
    stream << "        result.return_binding.return_surface};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeNativeReturnAdmission copperfin_runtime_bridge_admit_native_return(\n";
    stream << "    const CopperfinRuntimeBridgeInterpretedResultAdmission& interpreted_result_admission,\n";
    stream << "    const CopperfinRuntimeBridgeNativeReturnPlan& native_return_plan) {\n";
    stream << "    const auto& failure_policy_plan = native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan;\n";
    stream << "    const bool should_materialize_native_return = !interpreted_result_admission.should_use_fallback_return;\n";
    stream << "    const bool should_use_fallback_return = !should_materialize_native_return;\n";
    stream << "    return CopperfinRuntimeBridgeNativeReturnAdmission{\n";
    stream << "        should_materialize_native_return,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        should_use_fallback_return ? failure_policy_plan.diagnostics_fallback : interpreted_result_admission.diagnostics_value,\n";
    stream << "        failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeNativeReturn copperfin_runtime_bridge_execute_native_return(\n";
    stream << "    const CopperfinRuntimeBridgeNativeReturnPlan& plan) {\n";
    stream << "    const auto interpreted_result = copperfin_runtime_bridge_execute_interpreted_result(plan.interpreted_result_plan);\n";
    stream << "    const auto selected_value_representation = interpreted_result.matched_success_status\n";
    stream << "        ? plan.success_value_representation\n";
    stream << "        : plan.fallback_value_representation;\n";
    stream << "    const int selected_int_value = interpreted_result.matched_success_status\n";
    stream << "        ? plan.success_int_value\n";
    stream << "        : plan.fallback_int_value;\n";
    stream << "    return CopperfinRuntimeBridgeNativeReturn{\n";
    stream << "        interpreted_result.matched_success_status,\n";
    stream << "        std::move(selected_value_representation),\n";
    stream << "        selected_int_value,\n";
    stream << "        std::move(interpreted_result.diagnostics_value),\n";
    stream << "        interpreted_result.wrapper_return_surface};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_success_comparator_token() {\n";
    stream << "    return \" == \";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_fallback_comparator_token() {\n";
    stream << "    return \" != \";\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeOutcomeSelectionPlan copperfin_build_runtime_bridge_outcome_selection_plan(\n";
    stream << "    CopperfinRuntimeBridgeNativeReturnPlan native_return_plan) {\n";
    stream << "    const auto& interpreted_result_plan = native_return_plan.interpreted_result_plan;\n";
    stream << "    const auto& response_parse_plan = interpreted_result_plan.response_parse_plan;\n";
    stream << "    const auto success_comparator = copperfin_build_runtime_bridge_success_comparator_token();\n";
    stream << "    const auto fallback_comparator = copperfin_build_runtime_bridge_fallback_comparator_token();\n";
    stream << "    const auto success_condition =\n";
    stream << "        response_parse_plan.status_field + success_comparator + interpreted_result_plan.success_status_value;\n";
    stream << "    const auto fallback_condition =\n";
    stream << "        response_parse_plan.status_field + fallback_comparator + interpreted_result_plan.success_status_value;\n";
    stream << "    return CopperfinRuntimeBridgeOutcomeSelectionPlan{\n";
    stream << "        std::move(native_return_plan),\n";
    stream << "        std::move(success_condition),\n";
    stream << "        std::move(fallback_condition),\n";
    stream << "        response_parse_plan.diagnostics_field};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeOutcomeSelectionAdmission copperfin_runtime_bridge_admit_outcome_selection(\n";
    stream << "    const CopperfinRuntimeBridgeNativeReturnAdmission& native_return_admission,\n";
    stream << "    const CopperfinRuntimeBridgeOutcomeSelectionPlan& outcome_selection_plan) {\n";
    stream << "    const auto& failure_policy_plan = outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan;\n";
    stream << "    const bool should_select_outcome = !native_return_admission.should_use_fallback_return;\n";
    stream << "    const bool should_use_fallback_return = !should_select_outcome;\n";
    stream << "    return CopperfinRuntimeBridgeOutcomeSelectionAdmission{\n";
    stream << "        should_select_outcome,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        should_use_fallback_return ? failure_policy_plan.diagnostics_fallback : native_return_admission.diagnostics_value,\n";
    stream << "        failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeOutcomeSelection copperfin_runtime_bridge_execute_outcome_selection(\n";
    stream << "    const CopperfinRuntimeBridgeOutcomeSelectionPlan& plan) {\n";
    stream << "    const auto native_return = copperfin_runtime_bridge_execute_native_return(plan.native_return_plan);\n";
    stream << "    const auto selected_condition = native_return.matched_success_status\n";
    stream << "        ? plan.success_condition\n";
    stream << "        : plan.fallback_condition;\n";
    stream << "    return CopperfinRuntimeBridgeOutcomeSelection{\n";
    stream << "        native_return.matched_success_status,\n";
    stream << "        std::move(selected_condition),\n";
    stream << "        std::move(native_return.diagnostics_value),\n";
    stream << "        std::move(native_return.selected_value_representation),\n";
    stream << "        native_return.selected_int_value,\n";
    stream << "        native_return.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_native_int_return_surface() {\n";
    stream << "    return \"int\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_native_int_placeholder_signature_token() {\n";
    stream << "    return \"(int)\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_return_statement_from_expression(\n";
    stream << "    const std::string& value_expression) {\n";
    stream << "    return \"return \" + value_expression + \";\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_typed_native_return_expression(\n";
    stream << "    const std::string& native_return_surface,\n";
    stream << "    const std::string& int_value_representation) {\n";
    stream << "    const auto prefix = native_return_surface.substr(\n";
    stream << "        0U, native_return_surface.find(copperfin_build_runtime_bridge_native_int_placeholder_signature_token()));\n";
    stream << "    return prefix + \"(\" + int_value_representation + \")\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_return_statement(\n";
    stream << "    const std::string& native_return_surface,\n";
    stream << "    int int_value,\n";
    stream << "    const std::string& value_representation) {\n";
    stream << "    const auto int_value_representation = std::to_string(int_value);\n";
    stream << "    if (native_return_surface == copperfin_build_runtime_bridge_native_int_return_surface()) {\n";
    stream << "        return copperfin_build_runtime_bridge_return_statement_from_expression(int_value_representation);\n";
    stream << "    }\n";
    stream << "    const auto placeholder_index = native_return_surface.find(copperfin_build_runtime_bridge_native_int_placeholder_signature_token());\n";
    stream << "    if (placeholder_index != std::string::npos) {\n";
    stream << "        return copperfin_build_runtime_bridge_return_statement_from_expression(\n";
    stream << "            copperfin_build_runtime_bridge_typed_native_return_expression(native_return_surface, int_value_representation));\n";
    stream << "    }\n";
    stream << "    return copperfin_build_runtime_bridge_return_statement_from_expression(value_representation);\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_placeholder_return_statement(\n";
    stream << "    const CopperfinRuntimeBridgeReturn& return_binding) {\n";
    stream << "    return copperfin_build_runtime_bridge_return_statement(\n";
    stream << "        return_binding.return_surface,\n";
    stream << "        copperfin_parse_runtime_bridge_int_value_representation(return_binding.value_representation),\n";
    stream << "        return_binding.value_representation);\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnMaterializationPlan copperfin_build_runtime_bridge_return_materialization_plan(\n";
    stream << "    CopperfinRuntimeBridgeOutcomeSelectionPlan outcome_selection_plan) {\n";
    stream << "    const auto& native_return_plan = outcome_selection_plan.native_return_plan;\n";
    stream << "    const auto success_return_statement = copperfin_build_runtime_bridge_return_statement(\n";
    stream << "        native_return_plan.native_return_surface,\n";
    stream << "        native_return_plan.success_int_value,\n";
    stream << "        native_return_plan.success_value_representation);\n";
    stream << "    const auto fallback_return_statement = copperfin_build_runtime_bridge_return_statement(\n";
    stream << "        native_return_plan.native_return_surface,\n";
    stream << "        native_return_plan.fallback_int_value,\n";
    stream << "        native_return_plan.fallback_value_representation);\n";
    stream << "    return CopperfinRuntimeBridgeReturnMaterializationPlan{\n";
    stream << "        std::move(outcome_selection_plan),\n";
    stream << "        std::move(success_return_statement),\n";
    stream << "        std::move(fallback_return_statement),\n";
    stream << "        native_return_plan.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnMaterializationAdmission copperfin_runtime_bridge_admit_return_materialization(\n";
    stream << "    const CopperfinRuntimeBridgeOutcomeSelectionAdmission& outcome_selection_admission,\n";
    stream << "    const CopperfinRuntimeBridgeReturnMaterializationPlan& return_materialization_plan) {\n";
    stream << "    const auto& failure_policy_plan = return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan;\n";
    stream << "    const bool should_materialize_return = !outcome_selection_admission.should_use_fallback_return;\n";
    stream << "    const bool should_use_fallback_return = !should_materialize_return;\n";
    stream << "    return CopperfinRuntimeBridgeReturnMaterializationAdmission{\n";
    stream << "        should_materialize_return,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        should_use_fallback_return ? failure_policy_plan.diagnostics_fallback : outcome_selection_admission.diagnostics_value,\n";
    stream << "        failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnMaterialization copperfin_runtime_bridge_execute_return_materialization(\n";
    stream << "    const CopperfinRuntimeBridgeReturnMaterializationPlan& plan) {\n";
    stream << "    const auto outcome_selection = copperfin_runtime_bridge_execute_outcome_selection(plan.outcome_selection_plan);\n";
    stream << "    return CopperfinRuntimeBridgeReturnMaterialization{\n";
    stream << "        outcome_selection.matched_success_status,\n";
    stream << "        plan.outcome_selection_plan.success_condition,\n";
    stream << "        std::move(outcome_selection.selected_condition),\n";
    stream << "        std::move(outcome_selection.diagnostics_value),\n";
    stream << "        plan.success_return_statement,\n";
    stream << "        plan.fallback_return_statement,\n";
    stream << "        outcome_selection.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnEmissionPlan copperfin_build_runtime_bridge_return_emission_plan(\n";
    stream << "    CopperfinRuntimeBridgeReturnMaterializationPlan return_materialization_plan) {\n";
    stream << "    const auto return_materialization =\n";
    stream << "        copperfin_runtime_bridge_execute_return_materialization(return_materialization_plan);\n";
    stream << "    const auto success_branch_statement =\n";
    stream << "        \"if (\" + return_materialization.success_condition + \") { \"\n";
    stream << "        + return_materialization.success_return_statement + \" }\";\n";
    stream << "    const auto fallback_branch_statement =\n";
    stream << "        \"else { \" + return_materialization.fallback_return_statement + \" }\";\n";
    stream << "    const auto emitted_return_block =\n";
    stream << "        success_branch_statement + \" \" + fallback_branch_statement;\n";
    stream << "    return CopperfinRuntimeBridgeReturnEmissionPlan{\n";
    stream << "        std::move(return_materialization_plan),\n";
    stream << "        std::move(success_branch_statement),\n";
    stream << "        std::move(fallback_branch_statement),\n";
    stream << "        std::move(emitted_return_block)};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnEmissionAdmission copperfin_runtime_bridge_admit_return_emission(\n";
    stream << "    const CopperfinRuntimeBridgeReturnMaterializationAdmission& return_materialization_admission,\n";
    stream << "    const CopperfinRuntimeBridgeReturnEmissionPlan& return_emission_plan) {\n";
    stream << "    const bool should_emit_return = !return_materialization_admission.should_use_fallback_return;\n";
    stream << "    const bool should_use_fallback_return = !should_emit_return;\n";
    stream << "    return CopperfinRuntimeBridgeReturnEmissionAdmission{\n";
    stream << "        should_emit_return,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        return_materialization_admission.diagnostics_value,\n";
    stream << "        return_emission_plan.emitted_return_block};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnEmission copperfin_runtime_bridge_execute_return_emission(\n";
    stream << "    const CopperfinRuntimeBridgeReturnEmissionPlan& plan) {\n";
    stream << "    const auto return_materialization =\n";
    stream << "        copperfin_runtime_bridge_execute_return_materialization(plan.return_materialization_plan);\n";
    stream << "    return CopperfinRuntimeBridgeReturnEmission{\n";
    stream << "        return_materialization.matched_success_status,\n";
    stream << "        std::move(return_materialization.selected_condition),\n";
    stream << "        std::move(return_materialization.diagnostics_value),\n";
    stream << "        plan.success_branch_statement,\n";
    stream << "        plan.fallback_branch_statement,\n";
    stream << "        plan.emitted_return_block,\n";
    stream << "        return_materialization.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_replace_placeholder_return_mode() {\n";
    stream << "    return \"replace_placeholder_return\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_planned_activation_pending_mode() {\n";
    stream << "    return \"planned_activation_pending\";\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeFinalReturnAdoptionPlan copperfin_build_runtime_bridge_final_return_adoption_plan(\n";
    stream << "    CopperfinRuntimeBridgeReturnEmissionPlan return_emission_plan,\n";
    stream << "    std::string placeholder_return_statement) {\n";
    stream << "    const auto return_emission = copperfin_runtime_bridge_execute_return_emission(return_emission_plan);\n";
    stream << "    const auto adopted_return_block = return_emission.emitted_return_block;\n";
    stream << "    return CopperfinRuntimeBridgeFinalReturnAdoptionPlan{\n";
    stream << "        std::move(return_emission_plan),\n";
    stream << "        std::move(placeholder_return_statement),\n";
    stream << "        adopted_return_block,\n";
    stream << "        copperfin_build_runtime_bridge_replace_placeholder_return_mode()};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeFinalReturnAdoptionAdmission copperfin_runtime_bridge_admit_final_return_adoption(\n";
    stream << "    const CopperfinRuntimeBridgeReturnEmissionAdmission& return_emission_admission,\n";
    stream << "    const CopperfinRuntimeBridgeFinalReturnAdoptionPlan& final_return_adoption_plan) {\n";
    stream << "    const bool should_adopt_return = !return_emission_admission.should_use_fallback_return;\n";
    stream << "    const bool should_use_placeholder_return = !should_adopt_return;\n";
    stream << "    return CopperfinRuntimeBridgeFinalReturnAdoptionAdmission{\n";
    stream << "        should_adopt_return,\n";
    stream << "        should_use_placeholder_return,\n";
    stream << "        return_emission_admission.diagnostics_value,\n";
    stream << "        should_use_placeholder_return\n";
    stream << "            ? final_return_adoption_plan.placeholder_return_statement\n";
    stream << "            : final_return_adoption_plan.adopted_return_block};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeFinalReturnAdoption copperfin_runtime_bridge_execute_final_return_adoption(\n";
    stream << "    const CopperfinRuntimeBridgeFinalReturnAdoptionPlan& plan) {\n";
    stream << "    const auto return_emission = copperfin_runtime_bridge_execute_return_emission(plan.return_emission_plan);\n";
    stream << "    return CopperfinRuntimeBridgeFinalReturnAdoption{\n";
    stream << "        return_emission.matched_success_status,\n";
    stream << "        std::move(return_emission.selected_condition),\n";
    stream << "        std::move(return_emission.diagnostics_value),\n";
    stream << "        plan.placeholder_return_statement,\n";
    stream << "        plan.adopted_return_block,\n";
    stream << "        plan.adoption_mode,\n";
    stream << "        return_emission.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_activates_adopted_return_policy() {\n";
    stream << "    return false;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnActivationPlan copperfin_build_runtime_bridge_return_activation_plan(\n";
    stream << "    CopperfinRuntimeBridgeFinalReturnAdoptionPlan final_return_adoption_plan) {\n";
    stream << "    const auto final_return_adoption =\n";
    stream << "        copperfin_runtime_bridge_execute_final_return_adoption(final_return_adoption_plan);\n";
    stream << "    const auto active_return_block = final_return_adoption.adopted_return_block;\n";
    stream << "    return CopperfinRuntimeBridgeReturnActivationPlan{\n";
    stream << "        std::move(final_return_adoption_plan),\n";
    stream << "        copperfin_runtime_bridge_activates_adopted_return_policy(),\n";
    stream << "        copperfin_build_runtime_bridge_planned_activation_pending_mode(),\n";
    stream << "        active_return_block};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnActivationAdmission copperfin_runtime_bridge_admit_return_activation(\n";
    stream << "    const CopperfinRuntimeBridgeFinalReturnAdoptionAdmission& final_return_adoption_admission,\n";
    stream << "    const CopperfinRuntimeBridgeReturnActivationPlan& return_activation_plan) {\n";
    stream << "    const bool should_activate_return =\n";
    stream << "        return_activation_plan.activates_adopted_return && !final_return_adoption_admission.should_use_placeholder_return;\n";
    stream << "    const bool should_emit_placeholder_return = !should_activate_return;\n";
    stream << "    return CopperfinRuntimeBridgeReturnActivationAdmission{\n";
    stream << "        should_activate_return,\n";
    stream << "        should_emit_placeholder_return,\n";
    stream << "        final_return_adoption_admission.diagnostics_value,\n";
    stream << "        should_emit_placeholder_return\n";
    stream << "            ? return_activation_plan.final_return_adoption_plan.placeholder_return_statement\n";
    stream << "            : return_activation_plan.active_return_block};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnActivation copperfin_runtime_bridge_execute_return_activation(\n";
    stream << "    const CopperfinRuntimeBridgeReturnActivationPlan& plan) {\n";
    stream << "    const auto final_return_adoption =\n";
    stream << "        copperfin_runtime_bridge_execute_final_return_adoption(plan.final_return_adoption_plan);\n";
    stream << "    return CopperfinRuntimeBridgeReturnActivation{\n";
    stream << "        final_return_adoption.matched_success_status,\n";
    stream << "        std::move(final_return_adoption.selected_condition),\n";
    stream << "        std::move(final_return_adoption.diagnostics_value),\n";
    stream << "        plan.activates_adopted_return,\n";
    stream << "        plan.activation_mode,\n";
    stream << "        plan.active_return_block,\n";
    stream << "        final_return_adoption.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeStubReturnPlan copperfin_build_runtime_bridge_stub_return_plan(\n";
    stream << "    CopperfinRuntimeBridgeReturnActivationPlan return_activation_plan) {\n";
    stream << "    const auto return_activation =\n";
    stream << "        copperfin_runtime_bridge_execute_return_activation(return_activation_plan);\n";
    stream << "    const auto& final_return_adoption_plan = return_activation_plan.final_return_adoption_plan;\n";
    stream << "    const auto& native_return_plan = final_return_adoption_plan.return_emission_plan.return_materialization_plan\n";
    stream << "        .outcome_selection_plan.native_return_plan;\n";
    stream << "    const auto emitted_return_statement = return_activation.activates_adopted_return\n";
    stream << "        ? return_activation.active_return_block\n";
    stream << "        : final_return_adoption_plan.placeholder_return_statement;\n";
    stream << "    const auto deferred_return_block = return_activation.activates_adopted_return\n";
    stream << "        ? final_return_adoption_plan.placeholder_return_statement\n";
    stream << "        : return_activation.active_return_block;\n";
    stream << "    const bool emits_placeholder_return = !return_activation.activates_adopted_return;\n";
    stream << "    const auto activation_mode = return_activation.activation_mode;\n";
    stream << "    const auto adoption_mode = final_return_adoption_plan.adoption_mode;\n";
    stream << "    const bool keeps_placeholder_return_active =\n";
    stream << "        emits_placeholder_return || activation_mode == copperfin_build_runtime_bridge_planned_activation_pending_mode();\n";
    stream << "    const bool adopts_placeholder_replacement =\n";
    stream << "        adoption_mode == copperfin_build_runtime_bridge_replace_placeholder_return_mode();\n";
    stream << "    return CopperfinRuntimeBridgeStubReturnPlan{\n";
    stream << "        std::move(return_activation_plan),\n";
    stream << "        emitted_return_statement,\n";
    stream << "        deferred_return_block,\n";
    stream << "        emits_placeholder_return,\n";
    stream << "        activation_mode,\n";
    stream << "        adoption_mode,\n";
    stream << "        keeps_placeholder_return_active,\n";
    stream << "        adopts_placeholder_replacement,\n";
    stream << "        native_return_plan.fallback_int_value,\n";
    stream << "        native_return_plan.fallback_value_representation};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeStubReturnAdmission copperfin_runtime_bridge_admit_stub_return(\n";
    stream << "    const CopperfinRuntimeBridgeReturnActivationAdmission& return_activation_admission,\n";
    stream << "    const CopperfinRuntimeBridgeStubReturnPlan& stub_return_plan) {\n";
    stream << "    const bool should_route_stub_return = !return_activation_admission.should_activate_return;\n";
    stream << "    const bool should_emit_placeholder_return = stub_return_plan.emits_placeholder_return;\n";
    stream << "    return CopperfinRuntimeBridgeStubReturnAdmission{\n";
    stream << "        should_route_stub_return,\n";
    stream << "        should_emit_placeholder_return,\n";
    stream << "        return_activation_admission.diagnostics_value,\n";
    stream << "        should_emit_placeholder_return\n";
    stream << "            ? stub_return_plan.emitted_return_statement\n";
    stream << "            : stub_return_plan.deferred_return_block};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeStubReturn copperfin_runtime_bridge_execute_stub_return(\n";
    stream << "    const CopperfinRuntimeBridgeStubReturnPlan& plan) {\n";
    stream << "    const auto return_activation =\n";
    stream << "        copperfin_runtime_bridge_execute_return_activation(plan.return_activation_plan);\n";
    stream << "    const auto& final_return_adoption_plan = plan.return_activation_plan.final_return_adoption_plan;\n";
    stream << "    const auto& native_return_plan = final_return_adoption_plan.return_emission_plan.return_materialization_plan\n";
    stream << "        .outcome_selection_plan.native_return_plan;\n";
    stream << "    const auto emitted_return_statement = return_activation.activates_adopted_return\n";
    stream << "        ? return_activation.active_return_block\n";
    stream << "        : final_return_adoption_plan.placeholder_return_statement;\n";
    stream << "    const auto deferred_return_block = return_activation.activates_adopted_return\n";
    stream << "        ? final_return_adoption_plan.placeholder_return_statement\n";
    stream << "        : return_activation.active_return_block;\n";
    stream << "    const bool emits_placeholder_return = !return_activation.activates_adopted_return;\n";
    stream << "    const auto activation_mode = return_activation.activation_mode;\n";
    stream << "    const auto adoption_mode = final_return_adoption_plan.adoption_mode;\n";
    stream << "    const bool keeps_placeholder_return_active =\n";
    stream << "        emits_placeholder_return || activation_mode == copperfin_build_runtime_bridge_planned_activation_pending_mode();\n";
    stream << "    const bool adopts_placeholder_replacement =\n";
    stream << "        adoption_mode == copperfin_build_runtime_bridge_replace_placeholder_return_mode();\n";
    stream << "    return CopperfinRuntimeBridgeStubReturn{\n";
    stream << "        return_activation.matched_success_status,\n";
    stream << "        std::move(return_activation.selected_condition),\n";
    stream << "        std::move(return_activation.diagnostics_value),\n";
    stream << "        emitted_return_statement,\n";
    stream << "        deferred_return_block,\n";
    stream << "        emits_placeholder_return,\n";
    stream << "        activation_mode,\n";
    stream << "        adoption_mode,\n";
    stream << "        keeps_placeholder_return_active,\n";
    stream << "        adopts_placeholder_replacement,\n";
    stream << "        native_return_plan.fallback_int_value,\n";
    stream << "        native_return_plan.fallback_value_representation,\n";
    stream << "        return_activation.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgePlaceholderReturnValuePlan copperfin_build_runtime_bridge_placeholder_return_value_plan(\n";
    stream << "    CopperfinRuntimeBridgeStubReturnPlan stub_return_plan,\n";
    stream << "    CopperfinRuntimeBridgeStubEmissionWrapper stub_emission_wrapper) {\n";
    stream << "    const auto stub_return = copperfin_runtime_bridge_execute_stub_return(stub_return_plan);\n";
    stream << "    return CopperfinRuntimeBridgePlaceholderReturnValuePlan{\n";
    stream << "        std::move(stub_return_plan),\n";
    stream << "        stub_return.emits_placeholder_return,\n";
    stream << "        stub_return.emitted_return_statement,\n";
    stream << "        stub_return.deferred_return_block,\n";
    stream << "        stub_return.activation_mode,\n";
    stream << "        stub_return.adoption_mode,\n";
    stream << "        stub_return.keeps_placeholder_return_active,\n";
    stream << "        stub_return.adopts_placeholder_replacement,\n";
    stream << "        stub_return.placeholder_fallback_int_value,\n";
    stream << "        stub_return.placeholder_fallback_value_representation,\n";
    stream << "        std::move(stub_emission_wrapper)};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgePlaceholderReturnValueAdmission copperfin_runtime_bridge_admit_placeholder_return_value(\n";
    stream << "    const CopperfinRuntimeBridgeStubReturnAdmission& stub_return_admission,\n";
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnValuePlan& placeholder_return_value_plan) {\n";
    stream << "    const bool should_emit_placeholder_return = placeholder_return_value_plan.emits_placeholder_return;\n";
    stream << "    const bool should_keep_deferred_return = !should_emit_placeholder_return;\n";
    stream << "    return CopperfinRuntimeBridgePlaceholderReturnValueAdmission{\n";
    stream << "        should_emit_placeholder_return,\n";
    stream << "        should_keep_deferred_return,\n";
    stream << "        stub_return_admission.diagnostics_value,\n";
    stream << "        should_emit_placeholder_return\n";
    stream << "            ? placeholder_return_value_plan.emitted_return_statement\n";
    stream << "            : placeholder_return_value_plan.deferred_return_block};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgePlaceholderReturnValue copperfin_runtime_bridge_execute_placeholder_return_value(\n";
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnValuePlan& plan) {\n";
    stream << "    const auto stub_return = copperfin_runtime_bridge_execute_stub_return(plan.stub_return_plan);\n";
    stream << "    return CopperfinRuntimeBridgePlaceholderReturnValue{\n";
    stream << "        stub_return.matched_success_status,\n";
    stream << "        std::move(stub_return.selected_condition),\n";
    stream << "        std::move(stub_return.diagnostics_value),\n";
    stream << "        plan.emits_placeholder_return,\n";
    stream << "        plan.emitted_return_statement,\n";
    stream << "        plan.deferred_return_block,\n";
    stream << "        plan.activation_mode,\n";
    stream << "        plan.adoption_mode,\n";
    stream << "        plan.keeps_placeholder_return_active,\n";
    stream << "        plan.adopts_placeholder_replacement,\n";
    stream << "        plan.fallback_int_value,\n";
    stream << "        plan.fallback_value_representation,\n";
    stream << "        stub_return.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgePlaceholderReturnIntAdmission copperfin_runtime_bridge_admit_placeholder_return_int(\n";
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnValueAdmission& placeholder_return_value_admission,\n";
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnValuePlan& placeholder_return_value_plan) {\n";
    stream << "    return CopperfinRuntimeBridgePlaceholderReturnIntAdmission{\n";
    stream << "        true,\n";
    stream << "        placeholder_return_value_admission.should_emit_placeholder_return,\n";
    stream << "        placeholder_return_value_admission.diagnostics_value,\n";
    stream << "        placeholder_return_value_plan.fallback_int_value};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeStubEmissionAdmission copperfin_runtime_bridge_admit_stub_emission(\n";
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnIntAdmission& placeholder_return_int_admission) {\n";
    stream << "    return CopperfinRuntimeBridgeStubEmissionAdmission{\n";
    stream << "        placeholder_return_int_admission.should_emit_placeholder_return,\n";
    stream << "        placeholder_return_int_admission.diagnostics_value,\n";
    stream << "        placeholder_return_int_admission.selected_int_value};\n";
    stream << "}\n\n";
    stream << "static int copperfin_runtime_bridge_execute_placeholder_return_int(\n";
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnValue& placeholder_return_value) {\n";
    stream << "    (void)placeholder_return_value.emitted_return_statement;\n";
    stream << "    (void)placeholder_return_value.deferred_return_block;\n";
    stream << "    if (!placeholder_return_value.keeps_placeholder_return_active\n";
    stream << "        && placeholder_return_value.adopts_placeholder_replacement) {\n";
    stream << "        return placeholder_return_value.fallback_int_value;\n";
    stream << "    }\n";
    stream << "    return placeholder_return_value.fallback_int_value;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeStubEmission copperfin_runtime_bridge_execute_stub_emission(\n";
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnValuePlan& placeholder_return_value_plan) {\n";
    stream << "    const auto placeholder_return_value =\n";
    stream << "        copperfin_runtime_bridge_execute_placeholder_return_value(placeholder_return_value_plan);\n";
    stream << "    return CopperfinRuntimeBridgeStubEmission{\n";
    stream << "        placeholder_return_value.emits_placeholder_return,\n";
    stream << "        placeholder_return_value.diagnostics_value,\n";
    stream << "        copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_value)};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeStubEmissionReturnSurface copperfin_runtime_bridge_build_stub_emission_return_surface(\n";
    stream << "    const CopperfinRuntimeBridgeStubEmission& stub_emission,\n";
    stream << "    const std::string& native_return_surface) {\n";
    stream << "    return CopperfinRuntimeBridgeStubEmissionReturnSurface{\n";
    stream << "        native_return_surface,\n";
    stream << "        stub_emission.diagnostics_value,\n";
    stream << "        stub_emission.emitted_int_value};\n";
    stream << "}\n\n";
    stream << "static int copperfin_runtime_bridge_apply_stub_emission_output(\n";
    stream << "    const CopperfinRuntimeBridgeStubEmissionReturnSurface& stub_emission_return_surface,\n";
    stream << "    CopperfinRuntimeBridgeIntReturnAdapter return_adapter) {\n";
    stream << "    return return_adapter(stub_emission_return_surface.emitted_int_value);\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeStubEmissionWrapper copperfin_runtime_bridge_build_stub_emission_wrapper(\n";
    stream << "    const std::string& native_return_surface,\n";
    stream << "    CopperfinRuntimeBridgeIntReturnAdapter return_adapter) {\n";
    stream << "    return CopperfinRuntimeBridgeStubEmissionWrapper{\n";
    stream << "        native_return_surface,\n";
    stream << "        return_adapter};\n";
    stream << "}\n\n";
    stream << "static int copperfin_runtime_bridge_emit_stub_return_shared(\n";
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnValuePlan& placeholder_return_value_plan) {\n";
    stream << "    const auto stub_emission =\n";
    stream << "        copperfin_runtime_bridge_execute_stub_emission(placeholder_return_value_plan);\n";
    stream << "    const auto stub_emission_return_surface =\n";
    stream << "        copperfin_runtime_bridge_build_stub_emission_return_surface(\n";
    stream << "            stub_emission,\n";
    stream << "            placeholder_return_value_plan.stub_emission_wrapper.native_return_surface);\n";
    stream << "    return copperfin_runtime_bridge_apply_stub_emission_output(\n";
    stream << "        stub_emission_return_surface,\n";
    stream << "        placeholder_return_value_plan.stub_emission_wrapper.return_adapter);\n";
    stream << "}\n\n";

    if (plan.output_kind == BuildOutputKind::fll) {
        const auto parameter_counts = collect_library_export_parameter_counts(plan);
        const auto parameter_names = collect_library_export_parameter_names(plan);
        const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
        const auto routine_kinds = collect_library_export_routine_kinds(plan);
        const auto routine_locations = collect_library_export_routine_locations(plan);
        stream << "struct ParamBlk {\n";
        stream << "    int reserved = 0;\n";
        stream << "};\n\n";
        stream << "static std::string copperfin_build_runtime_bridge_fll_int_return_surface() {\n";
        stream << "    return \"" << kFllDefaultReturnHelper << "(int)\";\n";
        stream << "}\n\n";
        stream << "static int " << kFllDefaultReturnHelper << "(int value) {\n";
        stream << "    return value;\n";
        stream << "}\n\n";
        stream << "using CopperfinFllEntryPoint = int (*)(ParamBlk*);\n\n";
        stream << "struct CopperfinFoxInfoRecord {\n";
        stream << "    const char* function_name;\n";
        stream << "    CopperfinFllEntryPoint entrypoint;\n";
        stream << "    const char* routine_kind;\n";
        stream << "    const char* source_path;\n";
        stream << "    unsigned int source_line;\n";
        stream << "    const char* parameter_declaration_kind;\n";
        stream << "    const char* parameter_names;\n";
        stream << "    unsigned int parameter_count;\n";
        stream << "};\n\n";
        stream << "struct CopperfinFoxTableRecord {\n";
        stream << "    const CopperfinFoxTableRecord* previous;\n";
        stream << "    unsigned int entry_count;\n";
        stream << "    const CopperfinFoxInfoRecord* entries;\n";
        stream << "};\n\n";
        for (const auto& symbol : plan.exported_symbols) {
            const auto found = parameter_counts.find(symbol);
            const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
            const auto names_found = parameter_names.find(symbol);
            const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
            const auto kind_found = routine_kinds.find(symbol);
            const auto location_found = routine_locations.find(symbol);
            const std::string routine_kind =
                kind_found == routine_kinds.end() ? std::string("function") : kind_found->second;
            const SourceLocation location =
                location_found == routine_locations.end() ? SourceLocation{} : location_found->second;
            const std::string parameter_name_manifest =
                names_found == parameter_names.end()
                    ? std::string{}
                    : build_manifest_parameter_names(names_found->second);
            const std::string parameter_declaration_kind =
                declaration_kind_found == parameter_declaration_kinds.end()
                    ? std::string{}
                    : declaration_kind_found->second;
            stream << "COPPERFIN_EXPORT int " << symbol << "(ParamBlk* parm) {\n";
            stream << "    (void)parm;\n";
            stream << "    const auto descriptor = copperfin_build_runtime_bridge_descriptor(\""
                   << quote_manifest_value(symbol) << "\", \"" << quote_manifest_value(routine_kind)
                   << "\", \"" << quote_manifest_value(location.file_path) << "\", " << location.line
                   << "U, \"" << quote_manifest_value(parameter_declaration_kind) << "\", \""
                   << quote_manifest_value(parameter_name_manifest) << "\", " << parameter_count
                   << "U, reinterpret_cast<void*>(&" << symbol << "));\n";
            stream << "    const auto invocation = copperfin_build_runtime_bridge_invocation(descriptor);\n";
            stream << "    const auto call = copperfin_build_runtime_bridge_call(\n";
            stream << "        invocation,\n";
            stream << "        {{\"parm\", std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(parm))), \"ParamBlk*\"}});\n";
            stream << "    const auto placeholder_return_binding =\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_binding(\n";
            stream << "            copperfin_build_runtime_bridge_fll_int_return_surface());\n";
            stream << "    const auto result = copperfin_build_runtime_bridge_result(\n";
            stream << "        call,\n";
            stream << "        placeholder_return_binding);\n";
            stream << "    const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(result);\n";
            stream << "    const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(launch_plan);\n";
            stream << "    const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(observation_plan);\n";
            stream << "    const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(execution_plan);\n";
            stream << "    const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(transport_plan);\n";
            stream << "    const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(serialization_plan);\n";
            stream << "    const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);\n";
            stream << "    (void)dispatch_execution;\n";
            stream << "    const auto process_launch = copperfin_runtime_bridge_launch_process(dispatch_execution);\n";
            stream << "    (void)process_launch;\n";
            stream << "    const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(dispatch_plan);\n";
            stream << "    const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(\n";
            stream << "        payload_plan,\n";
            stream << "        copperfin_build_runtime_bridge_fll_int_return_surface());\n";
            stream << "    const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n";
            stream << "        interpretation_plan,\n";
            stream << "        placeholder_return_binding.value_representation);\n";
            stream << "    const auto host_failure =\n";
            stream << "        copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);\n";
            stream << "    (void)host_failure;\n";
            stream << "    const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n";
            stream << "        failure_policy);\n";
            stream << "    const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n";
            stream << "        response_validation);\n";
            stream << "    const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n";
            stream << "        request_artifact);\n";
            stream << "    const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n";
            stream << "        request_write_plan);\n";
            stream << "    const auto missing_response =\n";
            stream << "        copperfin_runtime_bridge_evaluate_missing_response(host_failure, response_read_plan);\n";
            stream << "    (void)missing_response;\n";
            stream << "    const auto response_validation_evaluation =\n";
            stream << "        copperfin_runtime_bridge_evaluate_response_validation(missing_response, response_validation);\n";
            stream << "    (void)response_validation_evaluation;\n";
            stream << "    const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n";
            stream << "        response_read_plan);\n";
            stream << "    const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n";
            stream << "        response_artifact);\n";
            stream << "    const auto response_parse_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);\n";
            stream << "    (void)response_parse_admission;\n";
            stream << "    const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n";
            stream << "        response_parse_plan);\n";
            stream << "    const auto interpreted_result_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);\n";
            stream << "    (void)interpreted_result_admission;\n";
            stream << "    const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n";
            stream << "        result,\n";
            stream << "        interpreted_result_plan);\n";
            stream << "    const auto native_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);\n";
            stream << "    (void)native_return_admission;\n";
            stream << "    const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n";
            stream << "        native_return_plan);\n";
            stream << "    const auto outcome_selection_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);\n";
            stream << "    (void)outcome_selection_admission;\n";
            stream << "    const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n";
            stream << "        outcome_selection_plan);\n";
            stream << "    const auto return_materialization_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);\n";
            stream << "    (void)return_materialization_admission;\n";
            stream << "    const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n";
            stream << "        return_materialization_plan);\n";
            stream << "    const auto return_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);\n";
            stream << "    (void)return_emission_admission;\n";
            stream << "    const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n";
            stream << "        return_emission_plan,\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));\n";
            stream << "    const auto final_return_adoption_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);\n";
            stream << "    (void)final_return_adoption_admission;\n";
            stream << "    const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n";
            stream << "        final_return_adoption_plan);\n";
            stream << "    const auto return_activation_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);\n";
            stream << "    (void)return_activation_admission;\n";
            stream << "    const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n";
            stream << "        return_activation_plan);\n";
            stream << "    const auto stub_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);\n";
            stream << "    (void)stub_return_admission;\n";
            stream << "    const auto stub_emission_wrapper =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_wrapper(\n";
            stream << "            copperfin_build_runtime_bridge_fll_int_return_surface(),\n";
            stream << "            " << kFllDefaultReturnHelper << ");\n";
            stream << "    const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n";
            stream << "        stub_return_plan,\n";
            stream << "        stub_emission_wrapper);\n";
            stream << "    const auto placeholder_return_value_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);\n";
            stream << "    (void)placeholder_return_value_admission;\n";
            stream << "    const auto placeholder_return_int_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);\n";
            stream << "    (void)placeholder_return_int_admission;\n";
            stream << "    const auto stub_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission);\n";
            stream << "    (void)stub_emission_admission;\n";
            stream << "    return copperfin_runtime_bridge_emit_stub_return_shared(placeholder_return_value_plan);\n";
            stream << "}\n\n";
        }
        stream << "static const CopperfinFoxInfoRecord kCopperfinFoxInfo[] = {\n";
        for (const auto& symbol : plan.exported_symbols) {
            const auto found = parameter_counts.find(symbol);
            const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
            const auto names_found = parameter_names.find(symbol);
            const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
            const auto kind_found = routine_kinds.find(symbol);
            const auto location_found = routine_locations.find(symbol);
            const std::string routine_kind =
                kind_found == routine_kinds.end() ? std::string("function") : kind_found->second;
            const SourceLocation location =
                location_found == routine_locations.end() ? SourceLocation{} : location_found->second;
            const std::string parameter_name_manifest =
                names_found == parameter_names.end()
                    ? std::string{}
                    : build_manifest_parameter_names(names_found->second);
            const std::string parameter_declaration_kind =
                declaration_kind_found == parameter_declaration_kinds.end()
                    ? std::string{}
                    : declaration_kind_found->second;
            stream << "    {\"" << symbol << "\", &" << symbol
                   << ", \"" << routine_kind << "\""
                   << ", \"" << quote_manifest_value(location.file_path) << "\""
                   << ", " << location.line << "U"
                   << ", \"" << parameter_declaration_kind << "\""
                   << ", \"" << parameter_name_manifest << "\""
                   << ", " << parameter_count << "U},\n";
        }
        stream << "};\n\n";
        stream << "COPPERFIN_EXPORT const CopperfinFoxTableRecord " << kFllRegistrationSymbol << " = {\n";
        stream << "    nullptr,\n";
        stream << "    static_cast<unsigned int>(sizeof(kCopperfinFoxInfo) / sizeof(kCopperfinFoxInfo[0])),\n";
        stream << "    kCopperfinFoxInfo\n";
        stream << "};\n\n";
        stream << "COPPERFIN_EXPORT const CopperfinFoxTableRecord* " << kFllLoaderEntrypoint << "() {\n";
        stream << "    return &" << kFllRegistrationSymbol << ";\n";
        stream << "}\n";
        return stream.str();
    }

    if (plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx) {
        const auto parameter_counts = collect_library_export_parameter_counts(plan);
        const auto parameter_names = collect_library_export_parameter_names(plan);
        const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
        const auto routine_kinds = collect_library_export_routine_kinds(plan);
        const auto routine_locations = collect_library_export_routine_locations(plan);
        stream << "#if defined(_WIN32) && defined(_M_IX86)\n";
        stream << "#define COPPERFIN_VFP_DLL_CALL __stdcall\n";
        stream << "#else\n";
        stream << "#define COPPERFIN_VFP_DLL_CALL\n";
        stream << "#endif\n\n";
        stream << "static int copperfin_runtime_bridge_return_native_int(int value) {\n";
        stream << "    return value;\n";
        stream << "}\n\n";
        for (const auto& symbol : plan.exported_symbols) {
            const auto found = parameter_counts.find(symbol);
            const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
            const auto names_found = parameter_names.find(symbol);
            const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
            const auto kind_found = routine_kinds.find(symbol);
            const auto location_found = routine_locations.find(symbol);
            const std::vector<std::string> effective_names =
                names_found == parameter_names.end()
                    ? std::vector<std::string>(parameter_count, std::string{})
                    : names_found->second;
            const std::string routine_kind =
                kind_found == routine_kinds.end() ? std::string("function") : kind_found->second;
            const SourceLocation location =
                location_found == routine_locations.end() ? SourceLocation{} : location_found->second;
            const std::string parameter_declaration_kind =
                declaration_kind_found == parameter_declaration_kinds.end()
                    ? std::string{}
                    : declaration_kind_found->second;
            const std::string parameter_name_manifest = build_manifest_parameter_names(effective_names);
            stream << "COPPERFIN_EXPORT int COPPERFIN_VFP_DLL_CALL " << symbol << "("
                   << build_placeholder_int_parameter_list(effective_names) << ") {\n";
            for (std::size_t index = 0; index < effective_names.size(); ++index) {
                stream << "    (void)" << sanitize_cpp_identifier(effective_names[index], index) << ";\n";
            }
            stream << "    const auto descriptor = copperfin_build_runtime_bridge_descriptor(\""
                   << quote_manifest_value(symbol) << "\", \"" << quote_manifest_value(routine_kind)
                   << "\", \"" << quote_manifest_value(location.file_path) << "\", " << location.line
                   << "U, \"" << quote_manifest_value(parameter_declaration_kind) << "\", \""
                   << quote_manifest_value(parameter_name_manifest) << "\", " << parameter_count
                   << "U, reinterpret_cast<void*>(&" << symbol << "));\n";
            stream << "    const auto invocation = copperfin_build_runtime_bridge_invocation(descriptor);\n";
            stream << "    const auto call = copperfin_build_runtime_bridge_call(\n";
            stream << "        invocation,\n";
            stream << "        {";
            for (std::size_t index = 0; index < effective_names.size(); ++index) {
                if (index > 0U) {
                    stream << ", ";
                }
                const std::string parameter_name = effective_names[index];
                const std::string sanitized_name = sanitize_cpp_identifier(effective_names[index], index);
                stream << "{\"" << quote_manifest_value(parameter_name) << "\", std::to_string(" << sanitized_name
                       << "), \"int\"}";
            }
            stream << "});\n";
            stream << "    const auto placeholder_return_binding =\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_binding(\"int\");\n";
            stream << "    const auto result = copperfin_build_runtime_bridge_result(call, placeholder_return_binding);\n";
            stream << "    const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(result);\n";
            stream << "    const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(launch_plan);\n";
            stream << "    const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(observation_plan);\n";
            stream << "    const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(execution_plan);\n";
            stream << "    const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(transport_plan);\n";
            stream << "    const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(serialization_plan);\n";
            stream << "    const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);\n";
            stream << "    (void)dispatch_execution;\n";
            stream << "    const auto process_launch = copperfin_runtime_bridge_launch_process(dispatch_execution);\n";
            stream << "    (void)process_launch;\n";
            stream << "    const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(dispatch_plan);\n";
            stream << "    const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(\n";
            stream << "        payload_plan,\n";
            stream << "        copperfin_build_runtime_bridge_native_int_return_surface());\n";
            stream << "    const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n";
            stream << "        interpretation_plan,\n";
            stream << "        placeholder_return_binding.value_representation);\n";
            stream << "    const auto host_failure =\n";
            stream << "        copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);\n";
            stream << "    (void)host_failure;\n";
            stream << "    const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n";
            stream << "        failure_policy);\n";
            stream << "    const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n";
            stream << "        response_validation);\n";
            stream << "    const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n";
            stream << "        request_artifact);\n";
            stream << "    const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n";
            stream << "        request_write_plan);\n";
            stream << "    const auto missing_response =\n";
            stream << "        copperfin_runtime_bridge_evaluate_missing_response(host_failure, response_read_plan);\n";
            stream << "    (void)missing_response;\n";
            stream << "    const auto response_validation_evaluation =\n";
            stream << "        copperfin_runtime_bridge_evaluate_response_validation(missing_response, response_validation);\n";
            stream << "    (void)response_validation_evaluation;\n";
            stream << "    const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n";
            stream << "        response_read_plan);\n";
            stream << "    const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n";
            stream << "        response_artifact);\n";
            stream << "    const auto response_parse_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);\n";
            stream << "    (void)response_parse_admission;\n";
            stream << "    const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n";
            stream << "        response_parse_plan);\n";
            stream << "    const auto interpreted_result_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);\n";
            stream << "    (void)interpreted_result_admission;\n";
            stream << "    const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n";
            stream << "        result,\n";
            stream << "        interpreted_result_plan);\n";
            stream << "    const auto native_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);\n";
            stream << "    (void)native_return_admission;\n";
            stream << "    const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n";
            stream << "        native_return_plan);\n";
            stream << "    const auto outcome_selection_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);\n";
            stream << "    (void)outcome_selection_admission;\n";
            stream << "    const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n";
            stream << "        outcome_selection_plan);\n";
            stream << "    const auto return_materialization_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);\n";
            stream << "    (void)return_materialization_admission;\n";
            stream << "    const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n";
            stream << "        return_materialization_plan);\n";
            stream << "    const auto return_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);\n";
            stream << "    (void)return_emission_admission;\n";
            stream << "    const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n";
            stream << "        return_emission_plan,\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));\n";
            stream << "    const auto final_return_adoption_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);\n";
            stream << "    (void)final_return_adoption_admission;\n";
            stream << "    const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n";
            stream << "        final_return_adoption_plan);\n";
            stream << "    const auto return_activation_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);\n";
            stream << "    (void)return_activation_admission;\n";
            stream << "    const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n";
            stream << "        return_activation_plan);\n";
            stream << "    const auto stub_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);\n";
            stream << "    (void)stub_return_admission;\n";
            stream << "    const auto stub_emission_wrapper =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_wrapper(\n";
            stream << "            copperfin_build_runtime_bridge_native_int_return_surface(),\n";
            stream << "            copperfin_runtime_bridge_return_native_int);\n";
            stream << "    const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n";
            stream << "        stub_return_plan,\n";
            stream << "        stub_emission_wrapper);\n";
            stream << "    const auto placeholder_return_value_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);\n";
            stream << "    (void)placeholder_return_value_admission;\n";
            stream << "    const auto placeholder_return_int_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);\n";
            stream << "    (void)placeholder_return_int_admission;\n";
            stream << "    const auto stub_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission);\n";
            stream << "    (void)stub_emission_admission;\n";
            stream << "    return copperfin_runtime_bridge_emit_stub_return_shared(placeholder_return_value_plan);\n";
            stream << "}\n\n";
        }
        return stream.str();
    }

    for (const auto& symbol : plan.exported_symbols) {
        stream << "COPPERFIN_EXPORT int " << symbol << "() {\n";
        stream << "    return -1;\n";
        stream << "}\n\n";
    }

    return stream.str();
}

std::string build_native_wrapper_cmake_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const std::string output_stem =
        std::filesystem::path(plan.launcher_output_path).stem().string();
    const std::string output_extension =
        std::filesystem::path(plan.launcher_output_path).extension().string();
    const std::string output_directory = "..";
    const std::string wrapper_file_name =
        std::filesystem::path(plan.native_wrapper_source_path).filename().string();
    const std::string module_definition_file_name =
        std::filesystem::path(plan.module_definition_path).filename().string();

    stream << "cmake_minimum_required(VERSION 3.20)\n";
    stream << "project(" << output_stem << "Wrapper LANGUAGES CXX)\n\n";
    stream << "add_library(" << output_stem << " SHARED " << wrapper_file_name << ")\n";
    stream << "target_compile_features(" << output_stem << " PRIVATE cxx_std_20)\n";
    stream << "set_target_properties(" << output_stem
           << " PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)\n";
    stream << "if(UNIX AND NOT APPLE)\n";
    stream << "  target_link_libraries(" << output_stem << " PRIVATE dl)\n";
    stream << "endif()\n";
    stream << "set_target_properties(" << output_stem
           << " PROPERTIES OUTPUT_NAME \"" << output_stem
           << "\" PREFIX \"\" SUFFIX \"" << output_extension
           << "\" LIBRARY_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/" << output_directory
           << "\" RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/" << output_directory
           << "\" ARCHIVE_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/" << output_directory << "\")\n";
    stream << "if(MSVC)\n";
    stream << "  target_link_options(" << output_stem
           << " PRIVATE \"/DEF:${CMAKE_CURRENT_SOURCE_DIR}/../" << module_definition_file_name << "\")\n";
    stream << "endif()\n";
    return stream.str();
}

std::string build_native_wrapper_shell_script_source() {
    std::ostringstream stream;
    stream << "#!/usr/bin/env sh\n";
    stream << "set -eu\n";
    stream << "SCRIPT_DIR=\"$(CDPATH= cd -- \"$(dirname -- \"$0\")\" && pwd)\"\n";
    stream << "cmake -S \"$SCRIPT_DIR\" -B \"$SCRIPT_DIR/build\"\n";
    stream << "cmake --build \"$SCRIPT_DIR/build\"\n";
    return stream.str();
}

std::string build_native_wrapper_powershell_script_source() {
    std::ostringstream stream;
    stream << "$ErrorActionPreference = 'Stop'\n";
    stream << "$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path\n";
    stream << "$buildDir = Join-Path $scriptDir 'build'\n";
    stream << "cmake -S $scriptDir -B $buildDir\n";
    stream << "cmake --build $buildDir\n";
    return stream.str();
}

std::string build_fll_api_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const auto parameter_counts = collect_library_export_parameter_counts(plan);
    const auto parameter_names = collect_library_export_parameter_names(plan);
    const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
    const auto routine_kinds = collect_library_export_routine_kinds(plan);
    const auto routine_locations = collect_library_export_routine_locations(plan);
    stream << "manifest_version=1\n";
    stream << "output_kind=fll\n";
    stream << "library_file=" << quote_manifest_value(std::filesystem::path(plan.launcher_output_path).filename().string()) << "\n";
    stream << "registration_model=FoxInfo/FoxTable\n";
    stream << "registration_command=SET LIBRARY TO\n";
    stream << "release_command=RELEASE LIBRARY\n";
    stream << "additive_supported=true\n";
    stream << "loader_entrypoint=" << kFllLoaderEntrypoint << "\n";
    stream << "registration_symbol=" << kFllRegistrationSymbol << "\n";
    stream << "callable_signature=" << kFllCallableSignature << "\n";
    stream << "default_return_helper=" << kFllDefaultReturnHelper << "\n";
    for (const auto& symbol : plan.exported_symbols) {
        stream << "function=" << quote_manifest_value(symbol) << "\n";
        const auto kind_found = routine_kinds.find(symbol);
        const auto location_found = routine_locations.find(symbol);
        stream << "function_kind="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(kind_found == routine_kinds.end() ? std::string("function") : kind_found->second) << "\n";
        stream << "function_source="
               << quote_manifest_value(symbol) << "|"
               << (location_found == routine_locations.end()
                       ? std::string{}
                       : build_manifest_source_location(location_found->second))
               << "\n";
        const auto found = parameter_counts.find(symbol);
        const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
        const auto names_found = parameter_names.find(symbol);
        const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
        stream << "function_arity="
               << quote_manifest_value(symbol) << "|"
               << parameter_count << "\n";
        stream << "function_parameter_declaration="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(
                      declaration_kind_found == parameter_declaration_kinds.end()
                          ? std::string{}
                          : declaration_kind_found->second)
               << "\n";
        stream << "function_parameters="
               << quote_manifest_value(symbol) << "|"
               << (names_found == parameter_names.end() ? std::string{} : build_manifest_parameter_names(names_found->second)) << "\n";
        stream << "function_call_surface="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(std::string(kFllCallableSignature)) << "|"
               << quote_manifest_value(std::string(kFllDefaultReturnHelper)) << "\n";
    }
    return stream.str();
}

std::string build_library_api_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const auto parameter_counts = collect_library_export_parameter_counts(plan);
    const auto parameter_names = collect_library_export_parameter_names(plan);
    const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
    const auto routine_kinds = collect_library_export_routine_kinds(plan);
    const auto routine_locations = collect_library_export_routine_locations(plan);
    stream << "manifest_version=1\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "library_file=" << quote_manifest_value(std::filesystem::path(plan.launcher_output_path).filename().string()) << "\n";
    stream << "callable_convention=" << kVfpLibraryCallableConvention << "\n";
    for (const auto& symbol : plan.exported_symbols) {
        const auto found = parameter_counts.find(symbol);
        const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
        const auto names_found = parameter_names.find(symbol);
        const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
        const auto kind_found = routine_kinds.find(symbol);
        const auto location_found = routine_locations.find(symbol);
        stream << "function=" << quote_manifest_value(symbol) << "\n";
        stream << "function_kind="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(kind_found == routine_kinds.end() ? std::string("function") : kind_found->second) << "\n";
        stream << "function_source="
               << quote_manifest_value(symbol) << "|"
               << (location_found == routine_locations.end()
                       ? std::string{}
                       : build_manifest_source_location(location_found->second))
               << "\n";
        stream << "function_arity="
               << quote_manifest_value(symbol) << "|"
               << parameter_count << "\n";
        stream << "function_parameter_declaration="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(
                      declaration_kind_found == parameter_declaration_kinds.end()
                          ? std::string{}
                          : declaration_kind_found->second)
               << "\n";
        stream << "function_parameters="
               << quote_manifest_value(symbol) << "|"
               << (names_found == parameter_names.end() ? std::string{} : build_manifest_parameter_names(names_found->second)) << "\n";
        stream << "function_call_surface="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(std::string(kVfpLibraryCallableConvention)) << "|"
               << quote_manifest_value(
                      names_found == parameter_names.end()
                          ? std::string{}
                          : build_placeholder_int_parameter_list(names_found->second))
               << "\n";
    }
    return stream.str();
}

void append_fxp_statement_lines(
    std::ostringstream& stream,
    const std::string& scope_name,
    const std::vector<Statement>& statements) {
    for (const auto& statement : statements) {
        stream << "statement="
               << quote_manifest_value(scope_name) << "|"
               << statement.location.line << "|"
               << quote_manifest_value(statement.text) << "\n";
    }
}

std::string build_fxp_token_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "manifest_version=1\n";
    stream << "output_kind=fxp\n";
    stream << "token_contract=logical_statements\n";
    stream << "primary_output=" << quote_manifest_value(std::filesystem::path(plan.launcher_output_path).filename().string()) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    for (const auto& asset : plan.assets) {
        if (lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string())) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        stream << "program=" << quote_manifest_value(asset.relative_path) << "\n";
        append_fxp_statement_lines(stream, "MAIN", program.main.statements);
        for (const auto& routine_entry : program.routines) {
            append_fxp_statement_lines(stream, routine_entry.first, routine_entry.second.statements);
        }
    }
    return stream.str();
}

bool write_fxp_primary_output_contract(
    const RuntimePackagePlan& plan,
    const std::string& token_manifest_text,
    std::string& error) {
    std::ostringstream stream;
    stream << "copperfin_fxp_contract_version=1\n";
    stream << "token_contract=copperfin_logical_statement_contract_v1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "token_manifest=" << quote_manifest_value(plan.fxp_token_manifest_path) << "\n";
    stream << token_manifest_text;
    return write_text_file(plan.launcher_output_path, stream.str(), error);
}

std::string build_app_archive_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "manifest_version=1\n";
    stream << "output_kind=app\n";
    stream << "archive_contract=staged_content_manifest\n";
    stream << "primary_output=" << quote_manifest_value(std::filesystem::path(plan.launcher_output_path).filename().string()) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    for (const auto& asset : plan.assets) {
        stream << "asset="
               << quote_manifest_value(asset.relative_path) << "|"
               << quote_manifest_value(asset.type_title) << "|"
               << (asset.required_for_runtime ? "true" : "false") << "|"
               << (asset.copied ? "true" : "false") << "\n";
    }
    return stream.str();
}

bool write_app_archive_primary_output(const RuntimePackagePlan& plan, std::string& error) {
    std::ostringstream stream;
    stream << "copperfin_app_archive_version=1\n";
    stream << "archive_contract=copperfin_content_archive_v1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "content_manifest=" << quote_manifest_value(plan.app_archive_manifest_path) << "\n";

    for (const auto& asset : plan.assets) {
        if (!asset.copied || trim_copy(asset.staged_path).empty() || !std::filesystem::exists(asset.staged_path)) {
            continue;
        }

        error.clear();
        const std::string bytes = read_binary_file(asset.staged_path, error);
        if (!error.empty()) {
            return false;
        }

        stream << "asset="
               << quote_manifest_value(asset.relative_path) << "|"
               << quote_manifest_value(asset.type_title) << "|"
               << (asset.required_for_runtime ? "true" : "false") << "|"
               << bytes.size() << "|"
               << quote_manifest_value(asset.sha256) << "\n";
        stream << "payload="
               << quote_manifest_value(asset.relative_path) << "|"
               << hex_encode_bytes(bytes) << "\n";
    }

    return write_text_file(plan.launcher_output_path, stream.str(), error);
}

void append_library_function_manifest_lines(std::ostringstream& stream, const RuntimePackagePlan& plan) {
    if (!is_library_output_kind(plan.output_kind)) {
        return;
    }

    const auto parameter_counts = collect_library_export_parameter_counts(plan);
    const auto parameter_names = collect_library_export_parameter_names(plan);
    const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
    const auto routine_kinds = collect_library_export_routine_kinds(plan);
    const auto routine_locations = collect_library_export_routine_locations(plan);
    for (const auto& symbol : plan.exported_symbols) {
        const auto found = parameter_counts.find(symbol);
        const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
        const auto names_found = parameter_names.find(symbol);
        const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
        const auto kind_found = routine_kinds.find(symbol);
        const auto location_found = routine_locations.find(symbol);
        stream << "library_function_kind="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(kind_found == routine_kinds.end() ? std::string("function") : kind_found->second) << "\n";
        stream << "library_function_source="
               << quote_manifest_value(symbol) << "|"
               << (location_found == routine_locations.end()
                       ? std::string{}
                       : build_manifest_source_location(location_found->second))
               << "\n";
        stream << "library_function_arity="
               << quote_manifest_value(symbol) << "|"
               << parameter_count << "\n";
        stream << "library_function_parameter_declaration="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(
                      declaration_kind_found == parameter_declaration_kinds.end()
                          ? std::string{}
                          : declaration_kind_found->second)
               << "\n";
        stream << "library_function_parameters="
               << quote_manifest_value(symbol) << "|"
               << (names_found == parameter_names.end() ? std::string{} : build_manifest_parameter_names(names_found->second)) << "\n";
        stream << "library_function_call_surface="
               << quote_manifest_value(symbol) << "|"
               << quote_manifest_value(
                      plan.output_kind == BuildOutputKind::fll
                          ? std::string(kFllCallableSignature)
                          : std::string(kVfpLibraryCallableConvention))
               << "|"
               << quote_manifest_value(
                      plan.output_kind == BuildOutputKind::fll
                          ? std::string(kFllDefaultReturnHelper)
                          : (names_found == parameter_names.end()
                                 ? std::string{}
                                 : build_placeholder_int_parameter_list(names_found->second)))
               << "\n";
    }
}

void append_runtime_asset_manifest_lines(std::ostringstream& stream, const RuntimePackagePlan& plan) {
    for (const auto& asset : plan.assets) {
        stream << "asset="
               << asset.record_index << "|"
               << quote_manifest_value(asset.relative_path) << "|"
               << quote_manifest_value(asset.staged_path) << "|"
               << quote_manifest_value(asset.type_title) << "|"
               << (asset.excluded ? "true" : "false") << "|"
               << (asset.exists ? "true" : "false") << "|"
               << quote_manifest_value(asset.sha256) << "|"
               << (asset.copied ? "true" : "false") << "\n";
    }
}

void append_warning_manifest_lines(std::ostringstream& stream, const RuntimePackagePlan& plan) {
    for (const auto& warning : plan.warnings) {
        stream << "warning=" << quote_manifest_value(warning) << "\n";
    }
}

void append_feature_flag_line(
    std::ostringstream& stream,
    std::string_view name,
    bool enabled,
    std::string_view category);

void append_runtime_feature_flag_manifest_lines(
    std::ostringstream& stream,
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile) {
    append_feature_flag_line(stream, "launcher.dotnet.requested", plan.requested_dotnet_launcher, "rollout");
    append_feature_flag_line(stream, "launcher.dotnet.active", plan.emit_dotnet_launcher, "host_compatibility");
    append_feature_flag_line(stream, "runtime.host.native", is_native_host_output_kind(plan.output_kind), "host_compatibility");
    append_feature_flag_line(stream, "build.output.ast_contract", true, "build_output");
    append_feature_flag_line(stream, "build.output.ir_contract", true, "build_output");
    append_feature_flag_line(stream, "build.output.csharp_transpilation", plan.requested_dotnet_launcher, "build_output");
    append_feature_flag_line(stream, "build.output.library_contract", is_library_output_kind(plan.output_kind), "build_output");
    append_feature_flag_line(stream, "build.output.native_library_wrapper", is_library_output_kind(plan.output_kind), "build_output");
    append_feature_flag_line(stream, "build.output.fll_api_contract", plan.output_kind == BuildOutputKind::fll, "build_output");
    append_feature_flag_line(stream, "build.output.fxp_token_contract", plan.output_kind == BuildOutputKind::fxp, "build_output");
    append_feature_flag_line(stream, "build.output.app_archive_contract", plan.output_kind == BuildOutputKind::app, "build_output");
    append_feature_flag_line(stream, "debug.breakpoints", plan.debug_plan.supports_breakpoints, "debug");
    append_feature_flag_line(stream, "debug.step_debugging", plan.debug_plan.supports_step_debugging, "debug");
    append_feature_flag_line(
        stream,
        "security.native",
        plan.security_enabled && security_profile.available,
        "security");
}

const char* statement_kind_name(const StatementKind kind) {
    switch (kind) {
        case StatementKind::assignment:
            return "assignment";
        case StatementKind::expression:
            return "expression";
        case StatementKind::do_command:
            return "do_command";
        case StatementKind::do_while_statement:
            return "do_while_statement";
        case StatementKind::do_case_statement:
            return "do_case_statement";
        case StatementKind::case_statement:
            return "case_statement";
        case StatementKind::otherwise_statement:
            return "otherwise_statement";
        case StatementKind::calculate_command:
            return "calculate_command";
        case StatementKind::count_command:
            return "count_command";
        case StatementKind::sum_command:
            return "sum_command";
        case StatementKind::average_command:
            return "average_command";
        case StatementKind::text_command:
            return "text_command";
        case StatementKind::total_command:
            return "total_command";
        case StatementKind::do_form:
            return "do_form";
        case StatementKind::report_form:
            return "report_form";
        case StatementKind::label_form:
            return "label_form";
        case StatementKind::activate_surface:
            return "activate_surface";
        case StatementKind::release_surface:
            return "release_surface";
        case StatementKind::return_statement:
            return "return_statement";
        case StatementKind::if_statement:
            return "if_statement";
        case StatementKind::else_statement:
            return "else_statement";
        case StatementKind::endif_statement:
            return "endif_statement";
        case StatementKind::for_statement:
            return "for_statement";
        case StatementKind::endfor_statement:
            return "endfor_statement";
        case StatementKind::loop_statement:
            return "loop_statement";
        case StatementKind::exit_statement:
            return "exit_statement";
        case StatementKind::enddo_statement:
            return "enddo_statement";
        case StatementKind::endcase_statement:
            return "endcase_statement";
        case StatementKind::read_events:
            return "read_events";
        case StatementKind::clear_events:
            return "clear_events";
        case StatementKind::begin_transaction:
            return "begin_transaction";
        case StatementKind::end_transaction:
            return "end_transaction";
        case StatementKind::rollback_transaction:
            return "rollback_transaction";
        case StatementKind::undo_command:
            return "undo_command";
        case StatementKind::seek_command:
            return "seek_command";
        case StatementKind::locate_command:
            return "locate_command";
        case StatementKind::scan_statement:
            return "scan_statement";
        case StatementKind::endscan_statement:
            return "endscan_statement";
        case StatementKind::replace_command:
            return "replace_command";
        case StatementKind::append_blank_command:
            return "append_blank_command";
        case StatementKind::delete_command:
            return "delete_command";
        case StatementKind::recall_command:
            return "recall_command";
        case StatementKind::pack_command:
            return "pack_command";
        case StatementKind::zap_command:
            return "zap_command";
        case StatementKind::unlock_command:
            return "unlock_command";
        case StatementKind::delete_from_command:
            return "delete_from_command";
        case StatementKind::insert_into_command:
            return "insert_into_command";
        case StatementKind::go_command:
            return "go_command";
        case StatementKind::skip_command:
            return "skip_command";
        case StatementKind::browse_command:
            return "browse_command";
        case StatementKind::select_command:
            return "select_command";
        case StatementKind::use_command:
            return "use_command";
        case StatementKind::set_order:
            return "set_order";
        case StatementKind::set_command:
            return "set_command";
        case StatementKind::set_library:
            return "set_library";
        case StatementKind::set_datasession:
            return "set_datasession";
        case StatementKind::set_default:
            return "set_default";
        case StatementKind::set_memowidth:
            return "set_memowidth";
        case StatementKind::on_error:
            return "on_error";
        case StatementKind::on_shutdown:
            return "on_shutdown";
        case StatementKind::with_statement:
            return "with_statement";
        case StatementKind::endwith_statement:
            return "endwith_statement";
        case StatementKind::try_statement:
            return "try_statement";
        case StatementKind::catch_statement:
            return "catch_statement";
        case StatementKind::finally_statement:
            return "finally_statement";
        case StatementKind::endtry_statement:
            return "endtry_statement";
        case StatementKind::public_declaration:
            return "public_declaration";
        case StatementKind::local_declaration:
            return "local_declaration";
        case StatementKind::private_declaration:
            return "private_declaration";
        case StatementKind::parameters_declaration:
            return "parameters_declaration";
        case StatementKind::lparameters_declaration:
            return "lparameters_declaration";
        case StatementKind::dimension_command:
            return "dimension_command";
        case StatementKind::store_command:
            return "store_command";
        case StatementKind::close_command:
            return "close_command";
        case StatementKind::erase_command:
            return "erase_command";
        case StatementKind::copy_file_command:
            return "copy_file_command";
        case StatementKind::rename_file_command:
            return "rename_file_command";
        case StatementKind::print_command:
            return "print_command";
        case StatementKind::create_cursor_command:
            return "create_cursor_command";
        case StatementKind::create_table_command:
            return "create_table_command";
        case StatementKind::alter_table_command:
            return "alter_table_command";
        case StatementKind::copy_to_command:
            return "copy_to_command";
        case StatementKind::append_from_command:
            return "append_from_command";
        case StatementKind::save_memvars_command:
            return "save_memvars_command";
        case StatementKind::restore_memvars_command:
            return "restore_memvars_command";
        case StatementKind::scatter_command:
            return "scatter_command";
        case StatementKind::gather_command:
            return "gather_command";
        case StatementKind::update_command:
            return "update_command";
        case StatementKind::retry_statement:
            return "retry_statement";
        case StatementKind::resume_statement:
            return "resume_statement";
        case StatementKind::declare_dll:
            return "declare_dll";
        case StatementKind::call_command:
            return "call_command";
        case StatementKind::for_each_statement:
            return "for_each_statement";
        case StatementKind::release_command:
            return "release_command";
        case StatementKind::clear_memory_command:
            return "clear_memory_command";
        case StatementKind::cancel_statement:
            return "cancel_statement";
        case StatementKind::quit_statement:
            return "quit_statement";
        case StatementKind::yield_statement:
            return "yield_statement";
        case StatementKind::doevents_command:
            return "doevents_command";
        case StatementKind::enter_critical_command:
            return "enter_critical_command";
        case StatementKind::exit_critical_command:
            return "exit_critical_command";
        case StatementKind::spawn_command:
            return "spawn_command";
        case StatementKind::await_command:
            return "await_command";
        case StatementKind::on_shutdown_statement:
            return "on_shutdown_statement";
        case StatementKind::edit_command:
            return "edit_command";
        case StatementKind::change_command:
            return "change_command";
        case StatementKind::input_command:
            return "input_command";
        case StatementKind::accept_command:
            return "accept_command";
        case StatementKind::getfile_command:
            return "getfile_command";
        case StatementKind::putfile_command:
            return "putfile_command";
        case StatementKind::getdir_command:
            return "getdir_command";
        case StatementKind::inputbox_command:
            return "inputbox_command";
        case StatementKind::wait_command:
            return "wait_command";
        case StatementKind::sleep_command:
            return "sleep_command";
        case StatementKind::keyboard_command:
            return "keyboard_command";
        case StatementKind::push_key_command:
            return "push_key_command";
        case StatementKind::pop_key_command:
            return "pop_key_command";
        case StatementKind::push_menu_command:
            return "push_menu_command";
        case StatementKind::pop_menu_command:
            return "pop_menu_command";
        case StatementKind::push_popup_command:
            return "push_popup_command";
        case StatementKind::pop_popup_command:
            return "pop_popup_command";
        case StatementKind::display_command:
            return "display_command";
        case StatementKind::list_command:
            return "list_command";
        case StatementKind::no_op:
            return "no_op";
    }
    return "no_op";
}

void append_ast_routine_json(
    std::ostringstream& stream,
    const std::string& routine_name,
    const std::vector<Statement>& statements) {
    stream << "        {\n";
    stream << "          \"name\": \"" << json_escape(routine_name) << "\",\n";
    stream << "          \"statements\": [\n";
    for (std::size_t index = 0; index < statements.size(); ++index) {
        const auto& statement = statements[index];
        stream << "            {\"line\": " << statement.location.line
               << ", \"text\": \"" << json_escape(statement.text) << "\""
               << ", \"identifier\": \"" << json_escape(statement.identifier) << "\""
               << ", \"expression\": \"" << json_escape(statement.expression) << "\"}";
        if (index + 1U != statements.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "          ]\n";
    stream << "        }";
}

std::string build_ast_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"schema_version\": 1,\n";
    stream << "  \"project_title\": \"" << json_escape(plan.project_title) << "\",\n";
    stream << "  \"output_kind\": \"" << json_escape(build_output_kind_name(plan.output_kind)) << "\",\n";
    stream << "  \"files\": [\n";

    bool first_file = true;
    for (const auto& asset : plan.assets) {
        if (lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string())) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        if (!first_file) {
            stream << ",\n";
        }
        first_file = false;
        stream << "    {\n";
        stream << "      \"relative_path\": \"" << json_escape(asset.relative_path) << "\",\n";
        stream << "      \"routines\": [\n";
        append_ast_routine_json(stream, "MAIN", program.main.statements);
        for (const auto& routine_entry : program.routines) {
            stream << ",\n";
            append_ast_routine_json(stream, routine_entry.first, routine_entry.second.statements);
        }
        stream << "\n";
        stream << "      ]\n";
        stream << "    }";
    }

    stream << "\n";
    stream << "  ]\n";
    stream << "}\n";
    return stream.str();
}

void append_ir_routine_json(
    std::ostringstream& stream,
    const std::string& routine_name,
    const std::vector<Statement>& statements) {
    stream << "        {\n";
    stream << "          \"name\": \"" << json_escape(routine_name) << "\",\n";
    stream << "          \"instructions\": [\n";
    for (std::size_t index = 0; index < statements.size(); ++index) {
        const auto& statement = statements[index];
        stream << "            {\"line\": " << statement.location.line
               << ", \"opcode\": \"" << json_escape(statement_kind_name(statement.kind)) << "\""
               << ", \"text\": \"" << json_escape(statement.text) << "\""
               << ", \"identifier\": \"" << json_escape(statement.identifier) << "\""
               << ", \"expression\": \"" << json_escape(statement.expression) << "\""
               << ", \"secondary_expression\": \"" << json_escape(statement.secondary_expression) << "\""
               << ", \"tertiary_expression\": \"" << json_escape(statement.tertiary_expression) << "\""
               << ", \"quaternary_expression\": \"" << json_escape(statement.quaternary_expression) << "\""
               << ", \"operand_count\": " << statement.names.size() << "}";
        if (index + 1U != statements.size()) {
            stream << ",";
        }
        stream << "\n";
    }
    stream << "          ]\n";
    stream << "        }";
}

std::string build_ir_manifest_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"schema_version\": 1,\n";
    stream << "  \"project_title\": \"" << json_escape(plan.project_title) << "\",\n";
    stream << "  \"output_kind\": \"" << json_escape(build_output_kind_name(plan.output_kind)) << "\",\n";
    stream << "  \"files\": [\n";

    bool first_file = true;
    for (const auto& asset : plan.assets) {
        if (lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string())) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        if (!first_file) {
            stream << ",\n";
        }
        first_file = false;
        stream << "    {\n";
        stream << "      \"relative_path\": \"" << json_escape(asset.relative_path) << "\",\n";
        stream << "      \"routines\": [\n";
        append_ir_routine_json(stream, "MAIN", program.main.statements);
        for (const auto& routine_entry : program.routines) {
            stream << ",\n";
            append_ir_routine_json(stream, routine_entry.first, routine_entry.second.statements);
        }
        stream << "\n";
        stream << "      ]\n";
        stream << "    }";
    }

    stream << "\n";
    stream << "  ]\n";
    stream << "}\n";
    return stream.str();
}

std::string sanitize_csharp_identifier(std::string value, const std::string& fallback) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return fallback;
    }

    std::string sanitized;
    sanitized.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        const unsigned char ch = static_cast<unsigned char>(value[index]);
        const bool valid =
            std::isalnum(ch) != 0 ||
            ch == '_';
        if (!valid) {
            sanitized.push_back('_');
            continue;
        }
        if (sanitized.empty() && std::isdigit(ch) != 0) {
            sanitized.push_back('_');
        }
        sanitized.push_back(static_cast<char>(ch));
    }
    return sanitized.empty() ? fallback : sanitized;
}

std::string sanitize_csharp_routine_identifier(std::string value, const std::string& fallback) {
    std::string sanitized = sanitize_csharp_identifier(std::move(value), fallback);
    for (char& ch : sanitized) {
        if (std::isalpha(static_cast<unsigned char>(ch)) != 0) {
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            break;
        }
    }
    return sanitized;
}

std::string unquote_literal(std::string value) {
    value = trim_copy(std::move(value));
    if (value.size() >= 2U &&
        ((value.front() == '\'' && value.back() == '\'') ||
         (value.front() == '"' && value.back() == '"'))) {
        value = value.substr(1U, value.size() - 2U);
    }
    return value;
}

std::string transpile_statement_to_csharp(
    const Statement& statement,
    const std::map<std::string, std::string>& routine_name_map) {
    switch (statement.kind) {
        case StatementKind::local_declaration: {
            std::ostringstream stream;
            for (const auto& name : statement.names) {
                stream << "dynamic " << sanitize_csharp_identifier(name, "localValue") << " = null;\n";
            }
            return stream.str();
        }
        case StatementKind::assignment:
            return statement.text + ";\n";
        case StatementKind::do_command: {
            const std::string routine_name = lowercase_copy(trim_copy(statement.identifier));
            const auto found = routine_name_map.find(routine_name);
            if (found != routine_name_map.end()) {
                return found->second + "();\n";
            }
            break;
        }
        case StatementKind::wait_command:
            if (!statement.expression.empty()) {
                return "Console.WriteLine(\"" + json_escape(unquote_literal(statement.expression)) + "\");\n";
            }
            break;
        case StatementKind::return_statement:
            return "return;\n";
        default:
            break;
    }

    return "throw new NotSupportedException(\"Unsupported FoxPro statement: " + json_escape(statement.text) + "\");\n";
}

std::string sanitize_csharp_compound_identifier(std::string value, const std::string& fallback) {
    value = trim_copy(std::move(value));
    if (value.empty()) {
        return fallback;
    }

    std::string sanitized;
    sanitized.reserve(value.size());
    bool capitalize_next = true;
    for (const char raw_ch : value) {
        const unsigned char ch = static_cast<unsigned char>(raw_ch);
        if (std::isalnum(ch) != 0) {
            char output = static_cast<char>(ch);
            if (capitalize_next && std::isalpha(ch) != 0) {
                output = static_cast<char>(std::toupper(ch));
            }
            if (sanitized.empty() && std::isdigit(ch) != 0) {
                sanitized.push_back('_');
            }
            sanitized.push_back(output);
            capitalize_next = false;
            continue;
        }

        if (!sanitized.empty() && sanitized.back() != '_') {
            sanitized.push_back('_');
        }
        capitalize_next = true;
    }

    while (!sanitized.empty() && sanitized.back() == '_') {
        sanitized.pop_back();
    }
    return sanitized.empty() ? fallback : sanitized;
}

std::string build_xasset_csharp_method_identifier(
    const XAssetExecutableModel& model,
    const XAssetMethod& method) {
    const std::string normalized_root = lowercase_copy(trim_copy(model.root_object_path));
    const std::string normalized_object = lowercase_copy(trim_copy(method.object_path));
    if (!normalized_root.empty() && normalized_object == normalized_root) {
        return sanitize_csharp_compound_identifier(method.method_name, "Method");
    }

    std::string method_prefix = method.object_path;
    if (!model.root_object_path.empty() &&
        normalized_object.size() > normalized_root.size() &&
        normalized_object.rfind(normalized_root + ".", 0U) == 0U) {
        method_prefix = method.object_path.substr(model.root_object_path.size() + 1U);
    }
    if (!method_prefix.empty()) {
        method_prefix += ".";
    }
    method_prefix += method.method_name;
    return sanitize_csharp_compound_identifier(method_prefix, "Method");
}

void append_xasset_csharp_type(
    std::ostringstream& stream,
    const studio::StudioDocumentModel& document) {
    const XAssetExecutableModel model = build_xasset_executable_model(document);
    if (!model.ok || model.root_object_path.empty()) {
        return;
    }

    const std::string type_name = sanitize_csharp_compound_identifier(model.root_object_path, "XAssetObject");
    std::map<std::string, std::string> method_name_map;
    for (const auto& method : model.methods) {
        method_name_map.emplace(method.routine_name, build_xasset_csharp_method_identifier(model, method));
    }

    stream << "    public sealed class " << type_name << "\n";
    stream << "    {\n";

    for (const auto& method : model.methods) {
        const auto mapped_name = method_name_map.find(method.routine_name);
        if (mapped_name == method_name_map.end()) {
            continue;
        }

        const std::string method_identity = method.object_path.empty()
            ? method.method_name
            : method.object_path + "." + method.method_name;
        stream << "        public void " << mapped_name->second << "()\n";
        stream << "        {\n";
        stream << "            throw new NotSupportedException(\"Manual port required for FoxPro xAsset method: "
               << json_escape(method_identity)
               << "\");\n";
        stream << "        }\n\n";
    }

    if (!model.startup_routines.empty()) {
        stream << "        public void RunStartup()\n";
        stream << "        {\n";
        for (const auto& routine_name : model.startup_routines) {
            const auto found = method_name_map.find(routine_name);
            if (found != method_name_map.end()) {
                stream << "            " << found->second << "();\n";
            }
        }
        stream << "        }\n\n";
    }

    if (!model.shutdown_routines.empty()) {
        stream << "        public void RunShutdown()\n";
        stream << "        {\n";
        for (const auto& routine_name : model.shutdown_routines) {
            const auto found = method_name_map.find(routine_name);
            if (found != method_name_map.end()) {
                stream << "            " << found->second << "();\n";
            }
        }
        stream << "        }\n\n";
    }

    stream << "    }\n\n";
}

std::string build_csharp_transpilation_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "using System;\n\n";
    stream << "namespace Copperfin.Generated\n";
    stream << "{\n";
    stream << "    public static class TranspiledProgram\n";
    stream << "    {\n";

    for (const auto& asset : plan.assets) {
        if (lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string())) != ".prg") {
            continue;
        }
        const Program program = parse_program(asset.source_path);
        std::map<std::string, std::string> routine_name_map;
        for (const auto& routine_entry : program.routines) {
            routine_name_map.emplace(lowercase_copy(routine_entry.first),
                                     sanitize_csharp_routine_identifier(routine_entry.first, "Routine"));
        }

        stream << "        public static void MainRoutine()\n";
        stream << "        {\n";
        for (const auto& statement : program.main.statements) {
            stream << "            " << transpile_statement_to_csharp(statement, routine_name_map);
        }
        stream << "        }\n\n";

        for (const auto& routine_entry : program.routines) {
            stream << "        public static void "
                   << sanitize_csharp_routine_identifier(routine_entry.first, "Routine")
                   << "()\n";
            stream << "        {\n";
            for (const auto& statement : routine_entry.second.statements) {
                stream << "            " << transpile_statement_to_csharp(statement, routine_name_map);
            }
            stream << "        }\n\n";
        }
    }

    stream << "    }\n\n";

    for (const auto& asset : plan.assets) {
        const std::string extension = lowercase_copy(trim_copy(std::filesystem::path(asset.source_path).extension().string()));
        if (extension != ".scx" && extension != ".vcx") {
            continue;
        }

        const auto open_result = studio::open_document({
            .path = asset.source_path,
            .load_full_table = true
        });
        if (!open_result.ok) {
            continue;
        }

        append_xasset_csharp_type(stream, open_result.document);
    }

    stream << "}\n";
    return stream.str();
}

std::string build_launcher_program_source(const RuntimePackagePlan&) {
    std::ostringstream stream;
    stream << "using System;\n";
    stream << "using System.Collections.Generic;\n";
    stream << "using System.Diagnostics;\n";
    stream << "using System.IO;\n\n";
    stream << "internal static class Program\n";
    stream << "{\n";
    stream << "    private static int Main(string[] args)\n";
    stream << "    {\n";
    stream << "        var baseDir = AppContext.BaseDirectory;\n";
    stream << "        var runtimeHost = Path.Combine(baseDir, \"copperfin_runtime_host.exe\");\n";
    stream << "        var manifest = Path.Combine(baseDir, \"app.cfmanifest\");\n";
    stream << "        if (!File.Exists(runtimeHost))\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(\"Copperfin runtime host was not found beside the launcher.\");\n";
    stream << "            return 3;\n";
    stream << "        }\n";
    stream << "        if (!File.Exists(manifest))\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(\"Copperfin manifest was not found beside the launcher.\");\n";
    stream << "            return 4;\n";
    stream << "        }\n\n";
    stream << "        var forwarded = new List<string> { \"--manifest\", Quote(manifest) };\n";
        stream << "        foreach (var arg in args)\n";
        stream << "        {\n";
        stream << "            if (string.Equals(arg, \"--debug\", StringComparison.OrdinalIgnoreCase) ||\n";
        stream << "                string.Equals(arg, \"/debug\", StringComparison.OrdinalIgnoreCase))\n";
        stream << "            {\n";
        stream << "                forwarded.Add(\"--debug\");\n";
        stream << "                continue;\n";
        stream << "            }\n";
        stream << "            forwarded.Add(Quote(arg));\n";
        stream << "        }\n\n";
    stream << "        var startInfo = new ProcessStartInfo\n";
    stream << "        {\n";
    stream << "            FileName = runtimeHost,\n";
    stream << "            Arguments = string.Join(\" \", forwarded),\n";
    stream << "            WorkingDirectory = baseDir,\n";
    stream << "            UseShellExecute = false\n";
    stream << "        };\n\n";
    stream << "        using var process = Process.Start(startInfo);\n";
    stream << "        if (process is null)\n";
    stream << "        {\n";
    stream << "            Console.Error.WriteLine(\"Copperfin runtime host could not be started.\");\n";
    stream << "            return 5;\n";
    stream << "        }\n";
    stream << "        process.WaitForExit();\n";
    stream << "        return process.ExitCode;\n";
    stream << "    }\n\n";
    stream << "    private static string Quote(string value)\n";
    stream << "    {\n";
    stream << "        return \"\\\"\" + value.Replace(\"\\\"\", \"\\\"\\\"\") + \"\\\"\";\n";
    stream << "    }\n";
    stream << "}\n";
    return stream.str();
}

std::string build_launcher_project_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    stream << "<Project Sdk=\"Microsoft.NET.Sdk\">\n";
    stream << "  <PropertyGroup>\n";
    stream << "    <OutputType>Exe</OutputType>\n";
    stream << "    <TargetFramework>net8.0-windows</TargetFramework>\n";
    stream << "    <ImplicitUsings>enable</ImplicitUsings>\n";
    stream << "    <Nullable>enable</Nullable>\n";
    stream << "    <UseWindowsForms>false</UseWindowsForms>\n";
    stream << "    <AssemblyName>" << sanitize_file_name(plan.project_title) << "</AssemblyName>\n";
    stream << "    <RootNamespace>Copperfin.Generated</RootNamespace>\n";
    stream << "    <PublishSingleFile>false</PublishSingleFile>\n";
    stream << "  </PropertyGroup>\n";
    stream << "</Project>\n";
    return stream.str();
}

bool copy_file_if_exists(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    std::string& error) {
    if (!std::filesystem::exists(source)) {
        error = "Source file does not exist: " + source.string();
        return false;
    }

    std::error_code directory_error;
    std::filesystem::create_directories(destination.parent_path(), directory_error);
    if (directory_error) {
        error = "Unable to create directory: " + destination.parent_path().string();
        return false;
    }

    std::error_code copy_error;
    std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
        error = "Unable to copy file to: " + destination.string();
        return false;
    }

    return true;
}

bool validate_runtime_host_source_path(
    const RuntimePackagePlan& plan,
    const std::string& runtime_host_source_path,
    std::string& error) {
    if (runtime_host_source_path.empty()) {
        error = "Runtime host source path is empty.";
        return false;
    }

    std::filesystem::path source(runtime_host_source_path);
    std::error_code canonical_error;
    source = std::filesystem::weakly_canonical(source, canonical_error);
    if (canonical_error) {
        error = "Unable to resolve runtime host source path.";
        return false;
    }

    if (!std::filesystem::exists(source) || !std::filesystem::is_regular_file(source)) {
        error = "Runtime host source path does not point to a regular file.";
        return false;
    }

    if (plan.security_enabled) {
#ifdef _WIN32
        const std::string expected_file_name = "copperfin_runtime_host.exe";
#else
        const std::string expected_file_name = "copperfin_runtime_host";
#endif
        if (!source.is_absolute()) {
            error = "Security-enabled packaging requires an absolute runtime host source path.";
            return false;
        }

        if (lowercase_copy(source.filename().string()) != expected_file_name) {
            error = "Security-enabled packaging requires canonical runtime host binary name.";
            return false;
        }
    }

    return true;
}

std::string resolve_project_item_source(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectEntry& entry) {
    const std::filesystem::path base_dir = std::filesystem::path(document.path).parent_path();

    if (!entry.relative_path.empty()) {
        const std::filesystem::path from_relative = base_dir / entry.relative_path;
        if (std::filesystem::exists(from_relative)) {
            return from_relative.lexically_normal().string();
        }
    }

    if (entry.name.empty()) {
        return {};
    }

    const std::filesystem::path raw(entry.name);
    if (raw.is_absolute()) {
        if (std::filesystem::exists(raw)) {
            return raw.lexically_normal().string();
        }

        const std::filesystem::path from_filename = base_dir / raw.filename();
        return from_filename.lexically_normal().string();
    }

    return (base_dir / raw).lexically_normal().string();
}

std::string relative_asset_path(const studio::StudioProjectEntry& entry) {
    const std::string path = !entry.relative_path.empty() ? entry.relative_path : entry.name;
    if (!path.empty()) {
        return path;
    }
    return "record_" + std::to_string(entry.record_index) + ".asset";
}

std::string resolve_working_directory(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace) {
    const std::filesystem::path document_dir = std::filesystem::path(document.path).parent_path();
    if (!workspace.home_directory.empty()) {
        const std::filesystem::path home_directory(workspace.home_directory);
        if (std::filesystem::exists(home_directory)) {
            return home_directory.lexically_normal().string();
        }
    }
    return document_dir.lexically_normal().string();
}

std::string resolve_security_role(bool security_enabled) {
    if (!security_enabled) {
        return {};
    }

    std::string role;
#ifdef _WIN32
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, "COPPERFIN_SECURITY_ROLE") == 0 && raw != nullptr) {
        role = raw;
        std::free(raw);
    }
#else
    if (const char* raw = std::getenv("COPPERFIN_SECURITY_ROLE"); raw != nullptr) {
        role = raw;
    }
#endif

    role = trim_copy(role);
    if (!role.empty()) {
        return role;
    }

    return "developer";
}

bool is_extension_payload_path(const std::filesystem::path& path) {
    const std::string extension = lowercase_copy(trim_copy(path.extension().string()));
    return extension == ".dll" || extension == ".exe" || extension == ".vsix";
}

bool is_recognized_security_role(
    const security::NativeSecurityProfile& profile,
    const std::string& role_id) {
    return std::find_if(profile.roles.begin(), profile.roles.end(), [&](const security::NativeRole& role) {
               return role.id == role_id;
           }) != profile.roles.end();
}

std::string join_strings(const std::vector<std::string>& values) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < values.size(); ++index) {
        stream << values[index];
        if ((index + 1U) != values.size()) {
            stream << ";";
        }
    }
    return stream.str();
}

bool is_prg_path(const std::string& value) {
    return lowercase_copy(trim_copy(std::filesystem::path(value).extension().string())) == ".prg";
}

void append_feature_flag_line(
    std::ostringstream& stream,
    std::string_view name,
    bool enabled,
    std::string_view category) {
    stream << "feature_flag="
           << name << "|"
           << (enabled ? "true" : "false") << "|"
           << category << "\n";
}

bool is_xasset_path(const std::string& value) {
    const std::string extension = trim_copy(std::filesystem::path(value).extension().string());
    return extension == ".scx" ||
        extension == ".vcx" ||
        extension == ".frx" ||
        extension == ".lbx" ||
        extension == ".mnx";
}

bool should_stage_asset(const RuntimePackageAsset& asset) {
    return asset.exists && (!asset.excluded || asset.required_for_runtime);
}

std::vector<std::filesystem::path> infer_companion_source_paths(const std::filesystem::path& source) {
    std::vector<std::filesystem::path> companions;
    const std::string extension = trim_copy(lowercase_copy(source.extension().string()));
    const auto same_stem = [&](const char* companion_extension) {
        auto path = source;
        path.replace_extension(companion_extension);
        return path;
    };

    if (extension == ".pjx") {
        companions.push_back(same_stem(".pjt"));
    } else if (extension == ".scx") {
        companions.push_back(same_stem(".sct"));
    } else if (extension == ".vcx") {
        companions.push_back(same_stem(".vct"));
    } else if (extension == ".frx") {
        companions.push_back(same_stem(".frt"));
    } else if (extension == ".lbx") {
        companions.push_back(same_stem(".lbt"));
    } else if (extension == ".mnx") {
        companions.push_back(same_stem(".mnt"));
    } else if (extension == ".dbf") {
        companions.push_back(same_stem(".fpt"));
        companions.push_back(same_stem(".cdx"));
        companions.push_back(same_stem(".idx"));
        companions.push_back(same_stem(".ndx"));
        companions.push_back(same_stem(".mdx"));
    } else if (extension == ".dbc") {
        companions.push_back(same_stem(".dct"));
        companions.push_back(same_stem(".dcx"));
    }

    return companions;
}

void copy_companion_files_if_present(
    const RuntimePackageAsset& asset,
    std::vector<std::string>& warnings) {
    const std::filesystem::path source(asset.source_path);
    const std::filesystem::path staged(asset.staged_path);
    for (const auto& companion_source : infer_companion_source_paths(source)) {
        if (!std::filesystem::exists(companion_source)) {
            continue;
        }

        auto companion_destination = staged;
        companion_destination.replace_extension(companion_source.extension().string());
        std::string error;
        if (!copy_file_if_exists(companion_source, companion_destination, error)) {
            warnings.push_back(error);
        }
    }
}

}  // namespace

const char* build_configuration_name(BuildConfiguration configuration) {
    switch (configuration) {
        case BuildConfiguration::debug:
            return "debug";
        case BuildConfiguration::release:
            return "release";
    }
    return "debug";
}

BuildConfiguration parse_build_configuration(const std::string& value) {
    return trim_copy(value) == "release"
        ? BuildConfiguration::release
        : BuildConfiguration::debug;
}

const char* build_output_kind_name(BuildOutputKind output_kind) {
    switch (output_kind) {
        case BuildOutputKind::executable:
            return "executable";
        case BuildOutputKind::app:
            return "app";
        case BuildOutputKind::dll:
            return "dll";
        case BuildOutputKind::fll:
            return "fll";
        case BuildOutputKind::fxp:
            return "fxp";
        case BuildOutputKind::ocx:
            return "ocx";
        case BuildOutputKind::unknown:
            return "unknown";
    }
    return "unknown";
}

RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher) {
    RuntimePackagePlan plan;
    plan.project_path = document.path;
    plan.project_title = workspace.project_title.empty()
        ? std::filesystem::path(document.path).stem().string()
        : workspace.project_title;
    plan.configuration = configuration;
    plan.security_enabled = enable_security;
    plan.output_kind = parse_build_output_kind(workspace.build_plan.output_kind);
    if (plan.output_kind == BuildOutputKind::unknown) {
        plan.output_kind = infer_build_output_kind_from_output_path(workspace.build_plan.output_path);
    }
    plan.requested_dotnet_launcher = emit_dotnet_launcher;
    plan.emit_dotnet_launcher =
        is_native_host_output_kind(plan.output_kind) &&
        emit_dotnet_launcher &&
        extensibility_profile.dotnet_output.available;
    if (is_library_output_kind(plan.output_kind)) {
        plan.launcher_mode = "foxpro_library_definition";
        plan.launcher_fallback = "library_binary_generation_pending";
    } else if (plan.output_kind == BuildOutputKind::app) {
        plan.launcher_mode = "foxpro_application_archive_contract";
        plan.launcher_fallback = "foxpro_app_binary_generation_pending";
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        plan.launcher_mode = "foxpro_tokenized_contract";
        plan.launcher_fallback = "foxpro_fxp_binary_generation_pending";
    } else {
        plan.launcher_mode = plan.emit_dotnet_launcher ? "dotnet_launcher" : "native_runtime_host";
        plan.launcher_fallback =
            (plan.requested_dotnet_launcher && !plan.emit_dotnet_launcher)
                ? "dotnet_output_unavailable"
                : "none";
    }

    if (!workspace.available) {
        plan.warnings.push_back("Project workspace is not available.");
        return plan;
    }

    const std::filesystem::path root(output_root);
    const std::filesystem::path package_root = root / sanitize_file_name(plan.project_title);
    const std::filesystem::path content_root = package_root / "content";
    plan.package_root = package_root.string();
    plan.content_root = content_root.string();
    plan.manifest_path = (package_root / "app.cfmanifest").string();
    plan.debug_manifest_path = (package_root / "app.cfdebug").string();
    plan.launcher_project_path = (package_root / "launcher" / "Copperfin.GeneratedLauncher.csproj").string();
    plan.launcher_source_path = (package_root / "launcher" / "Program.cs").string();
    const std::filesystem::path output_file_name(resolve_output_file_name(workspace, plan.project_title));
    plan.ast_manifest_path = (package_root / (output_file_name.string() + ".ast.json")).string();
    plan.ir_manifest_path = (package_root / (output_file_name.string() + ".ir.json")).string();
    plan.transpiled_csharp_path = (package_root / (output_file_name.string() + ".transpiled.cs")).string();
    std::filesystem::path module_definition_file_name = output_file_name;
    module_definition_file_name.replace_extension(".def");
    plan.launcher_output_path = (package_root / output_file_name).string();
    plan.module_definition_path = (package_root / module_definition_file_name).string();
    if (is_library_output_kind(plan.output_kind)) {
        const std::filesystem::path wrapper_root = package_root / "wrapper";
        const std::string output_stem = output_file_name.stem().string();
        plan.native_wrapper_source_path = (wrapper_root / (output_stem + "_wrapper.cpp")).string();
        plan.native_wrapper_cmake_path = (wrapper_root / "CMakeLists.txt").string();
        plan.native_wrapper_build_script_path = (wrapper_root / "build_wrapper.sh").string();
        plan.native_wrapper_build_powershell_path = (wrapper_root / "build_wrapper.ps1").string();
    }
    if (plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx) {
        std::filesystem::path library_api_manifest_file_name = output_file_name;
        library_api_manifest_file_name += ".api";
        plan.library_api_manifest_path = (package_root / library_api_manifest_file_name).string();
    }
    if (plan.output_kind == BuildOutputKind::fll) {
        std::filesystem::path fll_api_manifest_file_name = output_file_name;
        fll_api_manifest_file_name += ".api";
        plan.fll_api_manifest_path = (package_root / fll_api_manifest_file_name).string();
    }
    if (plan.output_kind == BuildOutputKind::fxp) {
        std::filesystem::path fxp_token_manifest_file_name = output_file_name;
        fxp_token_manifest_file_name += ".tokens";
        plan.fxp_token_manifest_path = (package_root / fxp_token_manifest_file_name).string();
    }
    if (plan.output_kind == BuildOutputKind::app) {
        std::filesystem::path app_archive_manifest_file_name = output_file_name;
        app_archive_manifest_file_name += ".contents";
        plan.app_archive_manifest_path = (package_root / app_archive_manifest_file_name).string();
    }
    plan.runtime_host_destination_path = (package_root / "copperfin_runtime_host.exe").string();
    plan.working_directory = content_root.lexically_normal().string();
    plan.startup_item = workspace.build_plan.startup_item;
    plan.security_role = resolve_security_role(enable_security);
    if (enable_security && !is_recognized_security_role(security_profile, plan.security_role)) {
        const std::string requested_role = plan.security_role;
        plan.security_role = "developer";
        if (!requested_role.empty()) {
            plan.warnings.push_back(
                "Unknown security role requested: " + requested_role + "; defaulting to developer.");
        }
    }
    plan.audit_log_path = (package_root / "security_audit.log").string();
    const std::string source_working_directory = resolve_working_directory(document, workspace);

    for (const auto& entry : workspace.entries) {
        RuntimePackageAsset asset;
        asset.record_index = entry.record_index;
        asset.relative_path = relative_asset_path(entry);
        asset.source_path = resolve_project_item_source(document, entry);
        asset.staged_path = (content_root / asset.relative_path).lexically_normal().string();
        asset.type_title = entry.type_title;
        asset.excluded = entry.excluded;
        asset.exists = !asset.source_path.empty() && std::filesystem::exists(asset.source_path);
        if (entry.record_index == workspace.build_plan.startup_record_index) {
            asset.required_for_runtime = true;
            plan.startup_source_path = asset.staged_path;
            plan.debug_plan.startup_source_path = asset.source_path;
        }
        if (!asset.exists && !entry.excluded && entry.group_id != "project") {
            plan.warnings.push_back("Missing project asset: " + asset.source_path);
        }
        plan.assets.push_back(std::move(asset));
    }

    if (plan.startup_source_path.empty()) {
        plan.warnings.push_back("No startup source asset could be resolved.");
    }
    if (plan.debug_plan.startup_source_path.empty()) {
        plan.warnings.push_back("No source-side startup asset could be resolved for debugging.");
    }

    plan.debug_plan.manifest_path = plan.debug_manifest_path;
    plan.debug_plan.startup_item = plan.startup_item;
    plan.debug_plan.working_directory = source_working_directory;
    plan.debug_plan.source_roots = unique_non_empty_paths_preserve_order({
        source_working_directory,
        plan.content_root
    });
    plan.debug_plan.supports_breakpoints =
        is_prg_path(plan.debug_plan.startup_source_path) ||
        is_xasset_path(plan.debug_plan.startup_source_path);
    plan.debug_plan.supports_step_debugging = plan.debug_plan.supports_breakpoints;

    if (enable_security && !security_profile.available) {
        plan.warnings.push_back("Security was requested but no native security profile is available.");
    }
    if (emit_dotnet_launcher && !extensibility_profile.dotnet_output.available) {
        plan.warnings.push_back(".NET launcher generation was requested but no .NET output profile is available.");
    }
    if (is_library_output_kind(plan.output_kind)) {
        plan.exported_symbols = collect_library_exported_symbols(plan);
        if (plan.exported_symbols.empty()) {
            plan.warnings.push_back("No PRG routine exports were discovered for the library output contract.");
        }
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        const bool has_prg_asset = std::any_of(plan.assets.begin(), plan.assets.end(), [](const RuntimePackageAsset& asset) {
            return is_prg_path(asset.source_path);
        });
        if (!has_prg_asset) {
            plan.warnings.push_back("No PRG sources were discovered for the FXP token contract.");
        }
    }

    plan.ok = true;
    return plan;
}

std::string build_runtime_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::ostringstream stream;
    stream << "manifest_version=1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "project_path=" << quote_manifest_value(plan.project_path) << "\n";
    stream << "package_root=" << quote_manifest_value(plan.package_root) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    stream << "working_directory=" << quote_manifest_value(plan.working_directory) << "\n";
    stream << "ast_manifest_path=" << quote_manifest_value(plan.ast_manifest_path) << "\n";
    stream << "ir_manifest_path=" << quote_manifest_value(plan.ir_manifest_path) << "\n";
    stream << "transpiled_csharp_path=" << quote_manifest_value(plan.transpiled_csharp_path) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "startup_source=" << quote_manifest_value(plan.startup_source_path) << "\n";
    stream << "configuration=" << build_configuration_name(plan.configuration) << "\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "primary_output_path=" << quote_manifest_value(plan.launcher_output_path) << "\n";
    stream << "primary_output_materialized=" << (plan.primary_output_materialized ? "true" : "false") << "\n";
    stream << "module_definition_path=" << quote_manifest_value(plan.module_definition_path) << "\n";
    stream << "native_wrapper_source_path=" << quote_manifest_value(plan.native_wrapper_source_path) << "\n";
    stream << "native_wrapper_cmake_path=" << quote_manifest_value(plan.native_wrapper_cmake_path) << "\n";
    stream << "native_wrapper_build_script_path=" << quote_manifest_value(plan.native_wrapper_build_script_path) << "\n";
    stream << "native_wrapper_build_powershell_path=" << quote_manifest_value(plan.native_wrapper_build_powershell_path) << "\n";
    stream << "library_api_manifest_path=" << quote_manifest_value(plan.library_api_manifest_path) << "\n";
    stream << "fll_api_manifest_path=" << quote_manifest_value(plan.fll_api_manifest_path) << "\n";
    stream << "fll_loader_entrypoint="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllLoaderEntrypoint) : std::string()) << "\n";
    stream << "fll_registration_symbol="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllRegistrationSymbol) : std::string()) << "\n";
    stream << "fll_callable_signature="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllCallableSignature) : std::string()) << "\n";
    stream << "fll_default_return_helper="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllDefaultReturnHelper) : std::string()) << "\n";
    stream << "library_callable_convention="
           << quote_manifest_value((plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx)
                                       ? std::string(kVfpLibraryCallableConvention)
                                       : std::string()) << "\n";
    stream << "fxp_token_manifest_path=" << quote_manifest_value(plan.fxp_token_manifest_path) << "\n";
    stream << "app_archive_manifest_path=" << quote_manifest_value(plan.app_archive_manifest_path) << "\n";
    stream << "security_enabled=" << (plan.security_enabled ? "true" : "false") << "\n";
    stream << "security_role=" << quote_manifest_value(plan.security_role) << "\n";
    stream << "security_mode=" << quote_manifest_value(security_profile.mode) << "\n";
    stream << "audit_log_path=" << quote_manifest_value(plan.audit_log_path) << "\n";
    stream << "runtime_host_sha256=" << quote_manifest_value(plan.runtime_host_sha256) << "\n";
    stream << "security_roles=" << security_profile.roles.size() << "\n";
    stream << "launcher_mode=" << quote_manifest_value(plan.launcher_mode) << "\n";
    stream << "launcher_fallback=" << quote_manifest_value(plan.launcher_fallback) << "\n";
    stream << "dotnet_enabled=" << (extensibility_profile.dotnet_output.available ? "true" : "false") << "\n";
    stream << "dotnet_story=" << quote_manifest_value(extensibility_profile.dotnet_output.primary_story) << "\n";
    stream << "dotnet_policy_allowlist=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    stream << "dotnet_policy_denylist=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    stream << "dotnet_parity_matrix_entries=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    stream << "dotnet_policy_allowlist_items=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.allowlist) {
        stream << "dotnet_policy_allowlist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_policy_denylist_items=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.denylist) {
        stream << "dotnet_policy_denylist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_parity_matrix_count=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    for (const auto& capability : extensibility_profile.dotnet_output.parity_matrix) {
        stream << "dotnet_parity_matrix_item="
               << quote_manifest_value(capability.id) << "|"
               << quote_manifest_value(capability.title) << "|"
               << dotnet_parity_tier_name(capability.tier) << "|"
               << quote_manifest_value(capability.rationale) << "|"
               << quote_manifest_value(capability.verification_reference) << "\n";
    }

    stream << "language_integration_count=" << extensibility_profile.languages.size() << "\n";
    for (const auto& language : extensibility_profile.languages) {
        stream << "language_integration="
               << quote_manifest_value(language.id) << "|"
               << quote_manifest_value(language.title) << "|"
               << quote_manifest_value(language.integration_mode) << "|"
               << quote_manifest_value(language.trust_boundary) << "|"
               << quote_manifest_value(language.output_story) << "|"
               << (language.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "ai_feature_count=" << extensibility_profile.ai_features.size() << "\n";
    for (const auto& feature : extensibility_profile.ai_features) {
        stream << "ai_feature="
               << quote_manifest_value(feature.id) << "|"
               << quote_manifest_value(feature.title) << "|"
               << quote_manifest_value(feature.description) << "|"
               << quote_manifest_value(feature.trust_boundary) << "|"
               << (feature.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "extensibility_guardrail_count=" << extensibility_profile.guardrails.size() << "\n";
    for (const auto& guardrail : extensibility_profile.guardrails) {
        stream << "extensibility_guardrail=" << quote_manifest_value(guardrail) << "\n";
    }

    const platform::DotNetInteropCallDecision launcher_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U,
            .requires_reflection = false,
            .untrusted_input = false,
            .security_sensitive = false});
    stream << "dotnet_gateway_task_primitives=" << quote_manifest_value(launcher_decision.execution_path + ":" + launcher_decision.reason) << "\n";

    const platform::DotNetInteropCallDecision denied_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 2U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true});
    stream << "dotnet_gateway_unsafe_reflection=" << quote_manifest_value(denied_decision.execution_path + ":" + denied_decision.reason) << "\n";

    stream << "language_integrations=" << extensibility_profile.languages.size() << "\n";
    stream << "ai_features=" << extensibility_profile.ai_features.size() << "\n";
    append_runtime_feature_flag_manifest_lines(stream, plan, security_profile);

    append_runtime_asset_manifest_lines(stream, plan);

    for (const auto& digest : plan.extension_payload_digests) {
        stream << "extension_payload="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }

    for (const auto& digest : plan.compiler_contract_digests) {
        stream << "compiler_contract="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }

    for (const auto& symbol : plan.exported_symbols) {
        stream << "export_symbol=" << quote_manifest_value(symbol) << "\n";
    }

    append_library_function_manifest_lines(stream, plan);

    append_warning_manifest_lines(stream, plan);

    return stream.str();
}

std::string build_debug_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::ostringstream stream;
    stream << "debug_manifest_version=1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "project_path=" << quote_manifest_value(plan.project_path) << "\n";
    stream << "package_root=" << quote_manifest_value(plan.package_root) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    stream << "ast_manifest_path=" << quote_manifest_value(plan.ast_manifest_path) << "\n";
    stream << "ir_manifest_path=" << quote_manifest_value(plan.ir_manifest_path) << "\n";
    stream << "transpiled_csharp_path=" << quote_manifest_value(plan.transpiled_csharp_path) << "\n";
    stream << "configuration=" << build_configuration_name(plan.configuration) << "\n";
    stream << "security_enabled=" << (plan.security_enabled ? "true" : "false") << "\n";
    stream << "security_role=" << quote_manifest_value(plan.security_role) << "\n";
    stream << "security_mode=" << quote_manifest_value(security_profile.mode) << "\n";
    stream << "audit_log_path=" << quote_manifest_value(plan.audit_log_path) << "\n";
    stream << "runtime_host_sha256=" << quote_manifest_value(plan.runtime_host_sha256) << "\n";
    stream << "security_roles=" << security_profile.roles.size() << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.debug_plan.startup_item) << "\n";
    stream << "startup_source=" << quote_manifest_value(plan.debug_plan.startup_source_path) << "\n";
    stream << "working_directory=" << quote_manifest_value(plan.debug_plan.working_directory) << "\n";
    stream << "supports_breakpoints=" << (plan.debug_plan.supports_breakpoints ? "true" : "false") << "\n";
    stream << "supports_step_debugging=" << (plan.debug_plan.supports_step_debugging ? "true" : "false") << "\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "primary_output_path=" << quote_manifest_value(plan.launcher_output_path) << "\n";
    stream << "primary_output_materialized=" << (plan.primary_output_materialized ? "true" : "false") << "\n";
    stream << "module_definition_path=" << quote_manifest_value(plan.module_definition_path) << "\n";
    stream << "native_wrapper_source_path=" << quote_manifest_value(plan.native_wrapper_source_path) << "\n";
    stream << "native_wrapper_cmake_path=" << quote_manifest_value(plan.native_wrapper_cmake_path) << "\n";
    stream << "native_wrapper_build_script_path=" << quote_manifest_value(plan.native_wrapper_build_script_path) << "\n";
    stream << "native_wrapper_build_powershell_path=" << quote_manifest_value(plan.native_wrapper_build_powershell_path) << "\n";
    stream << "library_api_manifest_path=" << quote_manifest_value(plan.library_api_manifest_path) << "\n";
    stream << "fll_api_manifest_path=" << quote_manifest_value(plan.fll_api_manifest_path) << "\n";
    stream << "fll_loader_entrypoint="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllLoaderEntrypoint) : std::string()) << "\n";
    stream << "fll_registration_symbol="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllRegistrationSymbol) : std::string()) << "\n";
    stream << "fll_callable_signature="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllCallableSignature) : std::string()) << "\n";
    stream << "fll_default_return_helper="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllDefaultReturnHelper) : std::string()) << "\n";
    stream << "library_callable_convention="
           << quote_manifest_value((plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx)
                                       ? std::string(kVfpLibraryCallableConvention)
                                       : std::string()) << "\n";
    stream << "launcher_mode=" << quote_manifest_value(plan.launcher_mode) << "\n";
    stream << "launcher_fallback=" << quote_manifest_value(plan.launcher_fallback) << "\n";
    stream << "dotnet_enabled=" << (extensibility_profile.dotnet_output.available ? "true" : "false") << "\n";
    stream << "dotnet_story=" << quote_manifest_value(extensibility_profile.dotnet_output.primary_story) << "\n";
    stream << "dotnet_policy_allowlist=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    stream << "dotnet_policy_denylist=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    stream << "dotnet_parity_matrix_entries=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    stream << "dotnet_policy_allowlist_items=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.allowlist) {
        stream << "dotnet_policy_allowlist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_policy_denylist_items=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.denylist) {
        stream << "dotnet_policy_denylist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_parity_matrix_count=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    for (const auto& capability : extensibility_profile.dotnet_output.parity_matrix) {
        stream << "dotnet_parity_matrix_item="
               << quote_manifest_value(capability.id) << "|"
               << quote_manifest_value(capability.title) << "|"
               << dotnet_parity_tier_name(capability.tier) << "|"
               << quote_manifest_value(capability.rationale) << "|"
               << quote_manifest_value(capability.verification_reference) << "\n";
    }
    const platform::DotNetInteropCallDecision launcher_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U,
            .requires_reflection = false,
            .untrusted_input = false,
            .security_sensitive = false});
    stream << "dotnet_gateway_task_primitives=" << quote_manifest_value(launcher_decision.execution_path + ":" + launcher_decision.reason) << "\n";

    const platform::DotNetInteropCallDecision denied_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 2U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true});
    stream << "dotnet_gateway_unsafe_reflection=" << quote_manifest_value(denied_decision.execution_path + ":" + denied_decision.reason) << "\n";
    stream << "language_integration_count=" << extensibility_profile.languages.size() << "\n";
    for (const auto& language : extensibility_profile.languages) {
        stream << "language_integration="
               << quote_manifest_value(language.id) << "|"
               << quote_manifest_value(language.title) << "|"
               << quote_manifest_value(language.integration_mode) << "|"
               << quote_manifest_value(language.trust_boundary) << "|"
               << quote_manifest_value(language.output_story) << "|"
               << (language.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "ai_feature_count=" << extensibility_profile.ai_features.size() << "\n";
    for (const auto& feature : extensibility_profile.ai_features) {
        stream << "ai_feature="
               << quote_manifest_value(feature.id) << "|"
               << quote_manifest_value(feature.title) << "|"
               << quote_manifest_value(feature.description) << "|"
               << quote_manifest_value(feature.trust_boundary) << "|"
               << (feature.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "extensibility_guardrail_count=" << extensibility_profile.guardrails.size() << "\n";
    for (const auto& guardrail : extensibility_profile.guardrails) {
        stream << "extensibility_guardrail=" << quote_manifest_value(guardrail) << "\n";
    }
    stream << "language_integrations=" << extensibility_profile.languages.size() << "\n";
    stream << "ai_features=" << extensibility_profile.ai_features.size() << "\n";
    stream << "source_roots=" << quote_manifest_value(join_strings(plan.debug_plan.source_roots)) << "\n";
    append_runtime_feature_flag_manifest_lines(stream, plan, security_profile);
    for (const auto& digest : plan.compiler_contract_digests) {
        stream << "compiler_contract="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }
    for (const auto& digest : plan.extension_payload_digests) {
        stream << "extension_payload="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }
    for (const auto& symbol : plan.exported_symbols) {
        stream << "export_symbol=" << quote_manifest_value(symbol) << "\n";
    }
    append_runtime_asset_manifest_lines(stream, plan);
    append_warning_manifest_lines(stream, plan);
    append_library_function_manifest_lines(stream, plan);
    return stream.str();
}

RuntimeMaterializeResult materialize_runtime_package(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& runtime_host_source_path) {
    if (!plan.ok) {
        return {.ok = false, .error = "Package plan is not valid."};
    }

    std::error_code directory_error;
    std::filesystem::create_directories(plan.package_root, directory_error);
    if (directory_error) {
        return {.ok = false, .error = "Unable to create package root."};
    }
    std::filesystem::create_directories(plan.content_root, directory_error);
    if (directory_error) {
        return {.ok = false, .error = "Unable to create content root."};
    }
    if (plan.emit_dotnet_launcher) {
        std::filesystem::create_directories(std::filesystem::path(plan.launcher_project_path).parent_path(), directory_error);
        if (directory_error) {
            return {.ok = false, .error = "Unable to create launcher directory."};
        }
    }

    RuntimePackagePlan materialized_plan = plan;
    std::string error;
    if (is_native_host_output_kind(plan.output_kind) &&
        !validate_runtime_host_source_path(plan, runtime_host_source_path, error)) {
        return {.ok = false, .error = error};
    }
    for (auto& asset : materialized_plan.assets) {
        if (!should_stage_asset(asset)) {
            continue;
        }

        const std::filesystem::path destination = std::filesystem::path(plan.content_root) / asset.relative_path;
        if (!copy_file_if_exists(asset.source_path, destination, error)) {
            materialized_plan.warnings.push_back(error);
            continue;
        }
        copy_companion_files_if_present(asset, materialized_plan.warnings);
        asset.copied = true;

        const auto digest = security::sha256_hex_for_file(destination.string());
        if (!digest.ok) {
            return {.ok = false, .error = digest.error};
        }
        asset.sha256 = digest.hex_digest;

        if (is_extension_payload_path(destination)) {
            materialized_plan.extension_payload_digests.push_back({
                .path = destination.string(),
                .sha256 = digest.hex_digest
            });
        }
    }

    if (is_library_output_kind(plan.output_kind)) {
        std::filesystem::create_directories(std::filesystem::path(plan.native_wrapper_source_path).parent_path(), directory_error);
        if (directory_error) {
            return {.ok = false, .error = "Unable to create native wrapper directory."};
        }
        if (!write_text_file(plan.module_definition_path, build_module_definition_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.module_definition_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_source_path, build_native_wrapper_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_source_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_cmake_path, build_native_wrapper_cmake_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_cmake_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_build_script_path, build_native_wrapper_shell_script_source(), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_build_script_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_build_powershell_path, build_native_wrapper_powershell_script_source(), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_build_powershell_path, error)) {
            return {.ok = false, .error = error};
        }
        if (plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx) {
            if (!write_text_file(plan.library_api_manifest_path, build_library_api_manifest_source(materialized_plan), error)) {
                return {.ok = false, .error = error};
            }
            if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.library_api_manifest_path, error)) {
                return {.ok = false, .error = error};
            }
        }
        if (plan.output_kind == BuildOutputKind::fll) {
            if (!write_text_file(plan.fll_api_manifest_path, build_fll_api_manifest_source(materialized_plan), error)) {
                return {.ok = false, .error = error};
            }
            if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.fll_api_manifest_path, error)) {
                return {.ok = false, .error = error};
            }
        }
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        const std::string fxp_token_manifest = build_fxp_token_manifest_source(materialized_plan);
        if (!write_text_file(plan.fxp_token_manifest_path, fxp_token_manifest, error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.fxp_token_manifest_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_fxp_primary_output_contract(materialized_plan, fxp_token_manifest, error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.extension_payload_digests, plan.launcher_output_path, error)) {
            return {.ok = false, .error = error};
        }
        materialized_plan.primary_output_materialized = true;
    } else if (plan.output_kind == BuildOutputKind::app) {
        if (!write_text_file(plan.app_archive_manifest_path, build_app_archive_manifest_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.app_archive_manifest_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_app_archive_primary_output(materialized_plan, error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.extension_payload_digests, plan.launcher_output_path, error)) {
            return {.ok = false, .error = error};
        }
        materialized_plan.primary_output_materialized = true;
    } else {
        if (!copy_file_if_exists(runtime_host_source_path, plan.runtime_host_destination_path, error)) {
            return {.ok = false, .error = error};
        }

        const auto runtime_host_digest = security::sha256_hex_for_file(plan.runtime_host_destination_path);
        if (!runtime_host_digest.ok) {
            return {.ok = false, .error = runtime_host_digest.error};
        }
        materialized_plan.runtime_host_sha256 = runtime_host_digest.hex_digest;
        materialized_plan.extension_payload_digests.push_back({
            .path = plan.runtime_host_destination_path,
            .sha256 = runtime_host_digest.hex_digest
        });

        if (!plan.emit_dotnet_launcher) {
            if (!copy_file_if_exists(plan.runtime_host_destination_path, plan.launcher_output_path, error)) {
                return {.ok = false, .error = error};
            }

            const auto native_entrypoint_digest = security::sha256_hex_for_file(plan.launcher_output_path);
            if (!native_entrypoint_digest.ok) {
                return {.ok = false, .error = native_entrypoint_digest.error};
            }
            materialized_plan.extension_payload_digests.push_back({
                .path = plan.launcher_output_path,
                .sha256 = native_entrypoint_digest.hex_digest
            });
            materialized_plan.primary_output_materialized = true;
        }
    }

    if (plan.emit_dotnet_launcher) {
        if (!write_text_file(plan.launcher_project_path, build_launcher_project_source(plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.launcher_source_path, build_launcher_program_source(plan), error)) {
            return {.ok = false, .error = error};
        }
    }

    if (!write_text_file(plan.ast_manifest_path, build_ast_manifest_source(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }
    if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.ast_manifest_path, error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.ir_manifest_path, build_ir_manifest_source(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }
    if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.ir_manifest_path, error)) {
        return {.ok = false, .error = error};
    }
    if (plan.requested_dotnet_launcher &&
        !write_text_file(plan.transpiled_csharp_path, build_csharp_transpilation_source(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }
    if (plan.requested_dotnet_launcher &&
        !append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.transpiled_csharp_path, error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.manifest_path, build_runtime_manifest_text(materialized_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.debug_manifest_path, build_debug_manifest_text(materialized_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }

    return {.ok = true, .plan = std::move(materialized_plan), .error = {}};
}

RuntimeBuildResult build_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    if (!plan.ok) {
        return {.ok = false, .error = "Package plan is not valid."};
    }
    if (!is_library_output_kind(plan.output_kind)) {
        return {.ok = false, .error = "Primary-output builds are only supported for library-output packages."};
    }
    if (!std::filesystem::exists(plan.native_wrapper_cmake_path)) {
        return {.ok = false, .error = "Native wrapper CMake metadata is missing."};
    }

    RuntimePackagePlan built_plan = plan;
    std::string error;
    const std::filesystem::path source_root = std::filesystem::path(plan.native_wrapper_cmake_path).parent_path();
    const std::filesystem::path build_root = source_root / "cmake_pipeline_build";
    const std::filesystem::path configure_log_path = build_root / "cmake-configure.log";
    const std::filesystem::path build_log_path = build_root / "cmake-build.log";
    std::error_code ignored;
    std::filesystem::remove_all(build_root, ignored);
    std::filesystem::remove(plan.launcher_output_path, ignored);
    std::filesystem::create_directories(build_root, ignored);
    if (ignored) {
        return {.ok = false, .error = "Unable to create native wrapper build directory."};
    }

    const std::string configure_command =
        "cmake -S \"" + source_root.string() + "\" -B \"" + build_root.string() + "\" > \"" +
        configure_log_path.string() + "\" 2>&1";
    if (std::system(configure_command.c_str()) != 0) {
        error = "native wrapper primary-output configure failed";
        if (std::filesystem::exists(configure_log_path)) {
            error += ":\n" + read_text_file(configure_log_path);
        }
        return {.ok = false, .error = error};
    }

    const std::string build_command =
        "cmake --build \"" + build_root.string() + "\" > \"" + build_log_path.string() + "\" 2>&1";
    if (std::system(build_command.c_str()) != 0) {
        error = "native wrapper primary-output build failed";
        if (std::filesystem::exists(build_log_path)) {
            error += ":\n" + read_text_file(build_log_path);
        }
        return {.ok = false, .error = error};
    }

    if (!std::filesystem::exists(plan.launcher_output_path)) {
        return {.ok = false, .error = "native wrapper primary-output build did not materialize the requested output path."};
    }

    built_plan.primary_output_materialized = true;
    if (!append_runtime_artifact_digest(built_plan.extension_payload_digests, plan.launcher_output_path, error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.manifest_path, build_runtime_manifest_text(built_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.debug_manifest_path, build_debug_manifest_text(built_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }

    return {.ok = true, .plan = std::move(built_plan), .error = {}};
}

}  // namespace copperfin::runtime
