// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"

#include <locale>

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

std::vector<std::string> collect_library_exported_symbols(const RuntimePackagePlan& plan) {
    std::vector<std::string> exported_symbols;
    std::unordered_set<std::string> seen;

    for (const auto& asset : plan.assets) {
        if (asset.excluded || !asset.exists) {
            continue;
        }

        if (lowercase_copy(copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(asset.source_path).extension())) != ".prg") {
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

    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
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

std::map<std::string, std::vector<std::string>> collect_library_export_parameter_names(const RuntimePackagePlan& plan) {
    std::map<std::string, std::vector<std::string>> parameter_names;
    std::unordered_set<std::string> seen;
    for (const auto& asset : plan.assets) {
        if (!asset.exists || asset.excluded) {
            continue;
        }

    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
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

    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
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

    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
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

        const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(asset.source_path).extension()));
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

            SourceLocation location = routine.declaration_location;
            if (!location.file_path.empty()) {
                location.file_path = copperfin::platform::path_to_utf8_string(
                    normalize_existing_path_spelling(
                        copperfin::platform::path_from_utf8_string(location.file_path)));
            }
            routine_locations.emplace(export_name, std::move(location));
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
    stream.imbue(std::locale::classic());
    stream << quote_manifest_value(location.file_path) << "|" << location.line;
    return stream.str();
}

std::string build_module_definition_source(const RuntimePackagePlan& plan) {
    std::ostringstream stream;
    const std::string output_stem =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.launcher_output_path).stem());
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
    stream.imbue(std::locale::classic());
    stream << "// Generated Copperfin native wrapper scaffold\n";
    stream << "// This is an honest bridge scaffold, not a finished FoxPro/VFP-compatible runtime wrapper.\n";
    stream << "#include <algorithm>\n";
    stream << "#include <array>\n";
    stream << "#include <atomic>\n";
    stream << "#include <cerrno>\n";
    stream << "#include <cstdint>\n";
    stream << "#include <cstring>\n";
    stream << "#include <cwchar>\n";
    stream << "#include <filesystem>\n";
    stream << "#include <fstream>\n";
    stream << "#include <iterator>\n";
    stream << "#include <mutex>\n";
    stream << "#include <sstream>\n";
    stream << "#include <string>\n";
    stream << "#include <utility>\n";
    stream << "#include <vector>\n";
    stream << "#if defined(_WIN32)\n";
    stream << "#include <windows.h>\n";
    stream << "#define COPPERFIN_EXPORT extern \"C\" __declspec(dllexport)\n";
    stream << "#else\n";
    stream << "#include <dlfcn.h>\n";
    stream << "#include <fcntl.h>\n";
    stream << "#include <sys/stat.h>\n";
    stream << "#include <sys/types.h>\n";
    stream << "#include <sys/wait.h>\n";
    stream << "#include <unistd.h>\n";
    stream << "extern char** environ;\n";
    stream << "#define COPPERFIN_EXPORT extern \"C\" __attribute__((visibility(\"default\")))\n";
    stream << "#endif\n\n";
    stream << R"CF(
static std::uint32_t copperfin_runtime_bridge_rotr(
    const std::uint32_t value,
    const unsigned int bits) {
    return (value >> bits) | (value << (32U - bits));
}

static std::string copperfin_runtime_bridge_sha256_bytes(
    const std::vector<std::uint8_t>& input) {
    static constexpr std::array<std::uint32_t, 64U> round_constants{
        0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
        0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
        0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
        0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
        0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
        0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
        0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
        0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
        0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
        0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
        0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
        0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
        0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
        0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
        0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
        0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U};
    std::vector<std::uint8_t> bytes = input;
    const std::uint64_t bit_length = static_cast<std::uint64_t>(bytes.size()) * 8ULL;
    bytes.push_back(0x80U);
    while ((bytes.size() % 64U) != 56U) {
        bytes.push_back(0U);
    }
    for (int shift = 56; shift >= 0; shift -= 8) {
        bytes.push_back(static_cast<std::uint8_t>((bit_length >> shift) & 0xffU));
    }

    std::array<std::uint32_t, 8U> state{
        0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
        0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U};
    for (std::size_t offset = 0; offset < bytes.size(); offset += 64U) {
        std::array<std::uint32_t, 64U> words{};
        for (std::size_t index = 0; index < 16U; ++index) {
            const std::size_t base = offset + index * 4U;
            words[index] = (static_cast<std::uint32_t>(bytes[base]) << 24U) |
                (static_cast<std::uint32_t>(bytes[base + 1U]) << 16U) |
                (static_cast<std::uint32_t>(bytes[base + 2U]) << 8U) |
                static_cast<std::uint32_t>(bytes[base + 3U]);
        }
        for (std::size_t index = 16U; index < words.size(); ++index) {
            const auto first = words[index - 15U];
            const auto second = words[index - 2U];
            const auto small_sigma_one = copperfin_runtime_bridge_rotr(second, 17U) ^
                copperfin_runtime_bridge_rotr(second, 19U) ^ (second >> 10U);
            const auto small_sigma_zero = copperfin_runtime_bridge_rotr(first, 7U) ^
                copperfin_runtime_bridge_rotr(first, 18U) ^ (first >> 3U);
            words[index] = words[index - 16U] + small_sigma_zero +
                words[index - 7U] + small_sigma_one;
        }

        std::uint32_t a = state[0];
        std::uint32_t b = state[1];
        std::uint32_t c = state[2];
        std::uint32_t d = state[3];
        std::uint32_t e = state[4];
        std::uint32_t f = state[5];
        std::uint32_t g = state[6];
        std::uint32_t h = state[7];
        for (std::size_t index = 0; index < words.size(); ++index) {
            const auto big_sigma_one = copperfin_runtime_bridge_rotr(e, 6U) ^
                copperfin_runtime_bridge_rotr(e, 11U) ^ copperfin_runtime_bridge_rotr(e, 25U);
            const auto choose = (e & f) ^ ((~e) & g);
            const auto temporary_one = h + big_sigma_one + choose +
                round_constants[index] + words[index];
            const auto big_sigma_zero = copperfin_runtime_bridge_rotr(a, 2U) ^
                copperfin_runtime_bridge_rotr(a, 13U) ^ copperfin_runtime_bridge_rotr(a, 22U);
            const auto majority = (a & b) ^ (a & c) ^ (b & c);
            const auto temporary_two = big_sigma_zero + majority;
            h = g;
            g = f;
            f = e;
            e = d + temporary_one;
            d = c;
            c = b;
            b = a;
            a = temporary_one + temporary_two;
        }
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }

    static constexpr char hex[] = "0123456789abcdef";
    std::string digest;
    digest.reserve(64U);
    for (const auto word : state) {
        for (int shift = 28; shift >= 0; shift -= 4) {
            digest.push_back(hex[(word >> shift) & 0x0fU]);
        }
    }
    return digest;
}

static std::string copperfin_runtime_bridge_manifest_value(
    const std::filesystem::path& manifest_path,
    const std::string& key) {
    std::ifstream input(manifest_path, std::ios::binary);
    if (!input) {
        return {};
    }
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

static bool copperfin_runtime_bridge_digest_matches(
    const std::filesystem::path& manifest_path,
    const std::vector<std::uint8_t>& bytes) {
    const auto expected = copperfin_runtime_bridge_manifest_value(
        manifest_path,
        "runtime_host_sha256");
    return !expected.empty() &&
        expected == copperfin_runtime_bridge_sha256_bytes(bytes);
}

#if defined(_WIN32)
static bool copperfin_runtime_bridge_read_verified_host(
    const std::filesystem::path& host_path,
    const std::filesystem::path& manifest_path,
    HANDLE& verified_host) {
    verified_host = INVALID_HANDLE_VALUE;
    const auto wide_path = host_path.wstring();
    const HANDLE handle = CreateFileW(
        wide_path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    const DWORD attributes = GetFileAttributesW(wide_path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) != 0U) {
        CloseHandle(handle);
        return false;
    }
    std::vector<std::uint8_t> bytes;
    std::array<std::uint8_t, 8192U> buffer{};
    for (;;) {
        DWORD read = 0U;
        if (!ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &read, nullptr)) {
            CloseHandle(handle);
            return false;
        }
        bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + read);
        if (read == 0U) {
            break;
        }
    }
    LARGE_INTEGER origin{};
    if (!SetFilePointerEx(handle, origin, nullptr, FILE_BEGIN) ||
        !copperfin_runtime_bridge_digest_matches(manifest_path, bytes)) {
        CloseHandle(handle);
        return false;
    }
    verified_host = handle;
    return true;
}
#else
static bool copperfin_runtime_bridge_read_verified_host(
    const std::filesystem::path& host_path,
    const std::filesystem::path& manifest_path,
    int& verified_host) {
    verified_host = -1;
    std::error_code status_error;
    const auto status = std::filesystem::symlink_status(host_path, status_error);
    if (status_error || status.type() != std::filesystem::file_type::regular) {
        return false;
    }
    const int descriptor = open(host_path.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (descriptor < 0) {
        return false;
    }
    struct stat before{};
    if (fstat(descriptor, &before) != 0 || !S_ISREG(before.st_mode) || before.st_nlink != 1) {
        close(descriptor);
        return false;
    }
    std::vector<std::uint8_t> bytes;
    std::array<std::uint8_t, 8192U> buffer{};
    for (;;) {
        const ssize_t read = ::read(descriptor, buffer.data(), buffer.size());
        if (read < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(descriptor);
            return false;
        }
        if (read == 0) {
            break;
        }
        bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + read);
    }
    struct stat after{};
    if (fstat(descriptor, &after) != 0 ||
        before.st_dev != after.st_dev || before.st_ino != after.st_ino ||
        before.st_size != after.st_size ||
        !copperfin_runtime_bridge_digest_matches(manifest_path, bytes)) {
        close(descriptor);
        return false;
    }
    verified_host = descriptor;
    return true;
}
#endif

)CF";
    stream << "static std::string copperfin_runtime_bridge_path_to_utf8_string(const std::filesystem::path& path) {\n";
    stream << "    const auto utf8 = path.u8string();\n";
    stream << "    return std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size());\n";
    stream << "}\n\n";
    stream << "static std::filesystem::path copperfin_wrapper_module_path(void* symbol_address) {\n";
    stream << "#if defined(_WIN32)\n";
    stream << "    HMODULE module = nullptr;\n";
    stream << "    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,\n";
    stream << "                            reinterpret_cast<LPCSTR>(symbol_address),\n";
    stream << "                            &module) || module == nullptr) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    std::wstring buffer(512U, L'\\0');\n";
    stream << "    for (;;) {\n";
    stream << "        const DWORD length = GetModuleFileNameW(module, buffer.data(), static_cast<DWORD>(buffer.size()));\n";
    stream << "        if (length == 0U) {\n";
    stream << "            return {};\n";
    stream << "        }\n";
    stream << "        if (length + 1U < buffer.size()) {\n";
    stream << "            buffer.resize(length);\n";
    stream << "            return std::filesystem::path(buffer).lexically_normal();\n";
    stream << "        }\n";
    stream << "        buffer.resize(buffer.size() * 2U);\n";
    stream << "    }\n";
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
    stream << "using CopperfinRuntimeBridgeIntReturnAdapter = int (*)(int);\n\n";
    stream << "struct CopperfinRuntimeBridgeStubEmissionWrapper {\n";
    stream << "    std::string native_return_surface;\n";
    stream << "    CopperfinRuntimeBridgeIntReturnAdapter return_adapter = nullptr;\n";
    stream << "};\n\n";
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
    stream << "    CopperfinRuntimeBridgeStubEmissionWrapper stub_emission_wrapper;\n";
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
    stream << "    std::filesystem::path manifest_path;\n";
    stream << "    std::vector<std::string> arguments;\n";
    stream << "    std::filesystem::path working_directory;\n";
    stream << "    std::vector<CopperfinRuntimeBridgeEnvironmentVariable> environment;\n";
    stream << "    std::filesystem::path stdout_log_path;\n";
    stream << "    std::filesystem::path stderr_log_path;\n";
    stream << "    bool capture_stdout = true;\n";
    stream << "    bool capture_stderr = true;\n";
    stream << "    int expected_exit_code = 0;\n";
    stream << "    bool verify_runtime_host = true;\n";
    stream << "};\n\n";
    stream << "struct CopperfinRuntimeBridgeProcessLaunch {\n";
    stream << "    std::filesystem::path executable_path;\n";
    stream << "    std::vector<std::string> arguments;\n";
    stream << "    std::filesystem::path working_directory;\n";
    stream << "    std::vector<CopperfinRuntimeBridgeEnvironmentVariable> environment;\n";
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
    stream << "    bool response_document_available = false;\n";
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
    stream << "    bool request_write_succeeded = false;\n";
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
    stream << "    CopperfinRuntimeBridgeParsedResponse parsed_response;\n";
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
    stream << "    CopperfinRuntimeBridgeInterpretedResult interpreted_result;\n";
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
    stream << "    CopperfinRuntimeBridgeNativeReturn native_return;\n";
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
    stream << "    CopperfinRuntimeBridgeOutcomeSelection outcome_selection;\n";
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
    stream << "    CopperfinRuntimeBridgeReturnMaterialization return_materialization;\n";
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
    stream << "    CopperfinRuntimeBridgeReturnEmission return_emission;\n";
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
    stream << "    CopperfinRuntimeBridgeFinalReturnAdoption final_return_adoption;\n";
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
    stream << "    CopperfinRuntimeBridgeReturnActivation return_activation;\n";
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
    stream << "struct CopperfinRuntimeBridgePlaceholderReturnValuePlan {\n";
    stream << "    CopperfinRuntimeBridgeStubReturnPlan stub_return_plan;\n";
    stream << "    CopperfinRuntimeBridgeStubReturn stub_return;\n";
    stream << "    bool emits_placeholder_return = true;\n";
    stream << "    std::string emitted_return_statement;\n";
    stream << "    std::string deferred_return_block;\n";
    stream << "    std::string activation_mode;\n";
    stream << "    std::string adoption_mode;\n";
    stream << "    bool keeps_placeholder_return_active = true;\n";
    stream << "    bool adopts_placeholder_replacement = true;\n";
    stream << "    int fallback_int_value = -1;\n";
    stream << "    std::string fallback_value_representation;\n";
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
    stream << "    void* symbol_address,\n";
    stream << "    CopperfinRuntimeBridgeStubEmissionWrapper stub_emission_wrapper) {\n";
    stream << "    return CopperfinRuntimeBridgeDescriptor{\n";
    stream << "        export_name,\n";
    stream << "        routine_kind,\n";
    stream << "        source_path,\n";
    stream << "        source_line,\n";
    stream << "        parameter_declaration_kind,\n";
    stream << "        parameter_names,\n";
    stream << "        parameter_count,\n";
    stream << "        copperfin_runtime_manifest_path(symbol_address),\n";
    stream << "        copperfin_runtime_host_path(symbol_address),\n";
    stream << "        std::move(stub_emission_wrapper)};\n";
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
    stream << "            copperfin_runtime_bridge_path_to_utf8_string(descriptor.runtime_host_path),\n";
    stream << "            copperfin_runtime_bridge_manifest_flag(),\n";
    stream << "            copperfin_runtime_bridge_path_to_utf8_string(descriptor.manifest_path),\n";
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
    stream << "static std::string copperfin_build_runtime_bridge_routine_kind_field_name() {\n";
    stream << "    return \"routine_kind\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_source_path_field_name() {\n";
    stream << "    return \"source_path\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_source_line_field_name() {\n";
    stream << "    return \"source_line\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_parameter_declaration_field_name() {\n";
    stream << "    return \"parameter_declaration\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_parameter_names_field_name() {\n";
    stream << "    return \"parameter_names\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_parameter_count_field_name() {\n";
    stream << "    return \"parameter_count\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_schema_version_field_name() {\n";
    stream << "    return \"schema_version\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_parameters_field_name() {\n";
    stream << "    return \"parameters\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_request_media_type_field_name() {\n";
    stream << "    return \"request_media_type\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_request_fields_field_name() {\n";
    stream << "    return \"request_fields\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_expected_response_media_type_field_name() {\n";
    stream << "    return \"expected_response_media_type\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_fields_field_name() {\n";
    stream << "    return \"response_fields\";\n";
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
    stream << "                   << copperfin_build_runtime_bridge_routine_kind_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.routine_kind)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_source_path_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.source_path)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_source_line_field_name()\n";
    stream << "                   << \"\\\": \" << call.invocation.descriptor.source_line << \",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_parameter_declaration_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.parameter_declaration_kind)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_parameter_names_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.parameter_names)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_parameter_count_field_name()\n";
    stream << "                   << \"\\\": \" << call.invocation.descriptor.parameter_count << \",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_schema_version_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(payload_plan.dispatch_plan.serialization_plan.schema_version)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_request_media_type_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(request_media_type)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_expected_response_media_type_field_name()\n";
    stream << "                   << \"\\\": \\\"\"\n";
    stream << "                   << copperfin_escape_runtime_bridge_json_string(payload_plan.dispatch_plan.serialization_plan.response_media_type)\n";
    stream << "                   << \"\\\",\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_request_fields_field_name()\n";
    stream << "                   << \"\\\": [\";\n";
    stream << "    for (std::size_t index = 0; index < payload_plan.request_fields.size(); ++index) {\n";
    stream << "        if (index > 0U) {\n";
    stream << "            request_stream << \", \";\n";
    stream << "        }\n";
    stream << "        request_stream << \"\\\"\"\n";
    stream << "                       << copperfin_escape_runtime_bridge_json_string(payload_plan.request_fields[index])\n";
    stream << "                       << \"\\\"\";\n";
    stream << "    }\n";
    stream << "    request_stream << \"],\\n\";\n";
    stream << "    request_stream << \"  \\\"\"\n";
    stream << "                   << copperfin_build_runtime_bridge_response_fields_field_name()\n";
    stream << "                   << \"\\\": [\";\n";
    stream << "    for (std::size_t index = 0; index < payload_plan.response_fields.size(); ++index) {\n";
    stream << "        if (index > 0U) {\n";
    stream << "            request_stream << \", \";\n";
    stream << "        }\n";
    stream << "        request_stream << \"\\\"\"\n";
    stream << "                       << copperfin_escape_runtime_bridge_json_string(payload_plan.response_fields[index])\n";
    stream << "                       << \"\\\"\";\n";
    stream << "    }\n";
    stream << "    request_stream << \"],\\n\";\n";
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
    stream << "    static std::atomic<unsigned long long> invocation_sequence{0};\n";
    stream << "    const auto sequence = ++invocation_sequence;\n";
    stream << "#if defined(_WIN32)\n";
    stream << "    const auto process_identity = static_cast<unsigned long long>(GetCurrentProcessId());\n";
    stream << "#else\n";
    stream << "    const auto process_identity = static_cast<unsigned long long>(getpid());\n";
    stream << "#endif\n";
    stream << "    const std::string invocation_identity = std::to_string(process_identity) + \"-\" + std::to_string(sequence);\n";
    stream << "    const std::string artifact_stem = std::string(export_name) + \".\" + invocation_identity;\n";
    stream << "    execution_plan.observation_plan.stdout_log_path =\n";
    stream << "        base_directory / (artifact_stem + copperfin_runtime_bridge_stdout_log_suffix());\n";
    stream << "    execution_plan.observation_plan.stderr_log_path =\n";
    stream << "        base_directory / (artifact_stem + copperfin_runtime_bridge_stderr_log_suffix());\n";
    stream << "    return CopperfinRuntimeBridgeTransportPlan{\n";
    stream << "        std::move(execution_plan),\n";
    stream << "        base_directory / (artifact_stem + copperfin_runtime_bridge_request_artifact_suffix()),\n";
    stream << "        base_directory / (artifact_stem + copperfin_runtime_bridge_response_artifact_suffix())};\n";
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
    stream << "    arguments.push_back(copperfin_runtime_bridge_path_to_utf8_string(serialization_plan.transport_plan.request_path));\n";
    stream << "    arguments.push_back(copperfin_build_runtime_bridge_response_path_argument_name());\n";
    stream << "    arguments.push_back(copperfin_runtime_bridge_path_to_utf8_string(serialization_plan.transport_plan.response_path));\n";
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
    stream << "        launch_plan.result.call.invocation.descriptor.manifest_path,\n";
    stream << "        plan.arguments,\n";
    stream << "        launch_plan.working_directory,\n";
    stream << "        launch_plan.environment,\n";
    stream << "        observation_plan.stdout_log_path,\n";
    stream << "        observation_plan.stderr_log_path,\n";
    stream << "        execution_plan.capture_stdout,\n";
    stream << "        execution_plan.capture_stderr,\n";
    stream << "        observation_plan.expected_exit_code,\n";
    stream << "        true};\n";
    stream << "}\n\n";
    stream << "#if defined(_WIN32)\n";
    stream << "static std::wstring copperfin_runtime_bridge_utf8_to_wide(const std::string& value) {\n";
    stream << "    if (value.empty()) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);\n";
    stream << "    if (count <= 0) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    std::wstring result(static_cast<std::size_t>(count), L'\\0');\n";
    stream << "    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), result.data(), count) != count) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    return result;\n";
    stream << "}\n\n";
    stream << "static std::wstring copperfin_runtime_bridge_quote_windows_argument(const std::wstring& argument) {\n";
    stream << "    const bool needs_quotes = argument.empty() || argument.find_first_of(L\" \\t\\r\\n\\\"\") != std::wstring::npos;\n";
    stream << "    if (!needs_quotes) {\n";
    stream << "        return argument;\n";
    stream << "    }\n";
    stream << "    std::wstring quoted(1U, L'\"');\n";
    stream << "    std::size_t backslashes = 0U;\n";
    stream << "    for (const wchar_t ch : argument) {\n";
    stream << "        if (ch == L'\\\\') {\n";
    stream << "            ++backslashes;\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        if (ch == L'\"') {\n";
    stream << "            quoted.append(backslashes * 2U + 1U, L'\\\\');\n";
    stream << "        } else {\n";
    stream << "            quoted.append(backslashes, L'\\\\');\n";
    stream << "        }\n";
    stream << "        quoted.push_back(ch);\n";
    stream << "        backslashes = 0U;\n";
    stream << "    }\n";
    stream << "    quoted.append(backslashes * 2U, L'\\\\');\n";
    stream << "    quoted.push_back(L'\"');\n";
    stream << "    return quoted;\n";
    stream << "}\n\n";
    stream << "static std::vector<std::wstring> copperfin_runtime_bridge_windows_environment(\n";
    stream << "    const std::vector<CopperfinRuntimeBridgeEnvironmentVariable>& overrides) {\n";
    stream << "    std::vector<std::wstring> entries;\n";
    stream << "    LPWCH raw_environment = GetEnvironmentStringsW();\n";
    stream << "    if (raw_environment != nullptr) {\n";
    stream << "        for (LPWCH current = raw_environment; *current != L'\\0'; current += std::wcslen(current) + 1U) {\n";
    stream << "            entries.emplace_back(current);\n";
    stream << "        }\n";
    stream << "        FreeEnvironmentStringsW(raw_environment);\n";
    stream << "    }\n";
    stream << "    for (const auto& override_value : overrides) {\n";
    stream << "        const auto name = copperfin_runtime_bridge_utf8_to_wide(override_value.name);\n";
    stream << "        const auto value = copperfin_runtime_bridge_utf8_to_wide(override_value.value);\n";
    stream << "        if (name.empty()) {\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        const std::wstring prefix = name + L\"=\";\n";
    stream << "        const std::wstring replacement = prefix + value;\n";
    stream << "        bool replaced = false;\n";
    stream << "        for (auto& entry : entries) {\n";
    stream << "            if (entry.size() >= prefix.size() && _wcsnicmp(entry.c_str(), prefix.c_str(), prefix.size()) == 0) {\n";
    stream << "                entry = replacement;\n";
    stream << "                replaced = true;\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        if (!replaced) {\n";
    stream << "            entries.push_back(replacement);\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    std::sort(entries.begin(), entries.end(), [](const std::wstring& left, const std::wstring& right) {\n";
    stream << "        const std::size_t left_separator = left.find(L'=');\n";
    stream << "        const std::size_t right_separator = right.find(L'=');\n";
    stream << "        return _wcsicmp(left.substr(0U, left_separator).c_str(), right.substr(0U, right_separator).c_str()) < 0;\n";
    stream << "    });\n";
    stream << "    return entries;\n";
    stream << "}\n\n";
    stream << "#else\n";
    stream << "static std::string copperfin_runtime_bridge_posix_environment_key(const std::string& entry) {\n";
    stream << "    const std::size_t separator = entry.find('=');\n";
    stream << "    return separator == std::string::npos ? entry : entry.substr(0U, separator);\n";
    stream << "}\n\n";
    stream << "static std::vector<std::string> copperfin_runtime_bridge_posix_environment(\n";
    stream << "    const std::vector<CopperfinRuntimeBridgeEnvironmentVariable>& overrides) {\n";
    stream << "    // Direct environ mutation must use the same external process-wide serialization contract.\n";
    stream << "    static std::mutex environment_mutex;\n";
    stream << "    const std::lock_guard<std::mutex> environment_lock(environment_mutex);\n";
    stream << "    std::vector<std::string> entries;\n";
    stream << "    for (char** current = environ; current != nullptr && *current != nullptr; ++current) {\n";
    stream << "        const std::string entry(*current);\n";
    stream << "        const std::string key = copperfin_runtime_bridge_posix_environment_key(entry);\n";
    stream << "        bool replaced = false;\n";
    stream << "        for (auto& existing : entries) {\n";
    stream << "            if (copperfin_runtime_bridge_posix_environment_key(existing) == key) {\n";
    stream << "                existing = entry;\n";
    stream << "                replaced = true;\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        if (!replaced) {\n";
    stream << "            entries.push_back(entry);\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    for (const auto& override_value : overrides) {\n";
    stream << "        const std::string prefix = override_value.name + \"=\";\n";
    stream << "        const std::string replacement = prefix + override_value.value;\n";
    stream << "        bool replaced = false;\n";
    stream << "        for (auto& entry : entries) {\n";
    stream << "            if (entry.compare(0U, prefix.size(), prefix) == 0) {\n";
    stream << "                entry = replacement;\n";
    stream << "                replaced = true;\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        if (!replaced) {\n";
    stream << "            entries.push_back(replacement);\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    return entries;\n";
    stream << "}\n\n";
    stream << "#endif\n\n";
    stream << "static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_launch_process(\n";
    stream << "    const CopperfinRuntimeBridgeDispatchExecution& dispatch_execution);\n\n";
    stream << "#if defined(COPPERFIN_RUNTIME_BRIDGE_TEST_HOOKS)\n";
    stream << "COPPERFIN_EXPORT int copperfin_runtime_bridge_test_launch_environment(\n";
    stream << "    const char* executable_path,\n";
    stream << "    const char* output_path,\n";
    stream << "    const char* working_directory,\n";
    stream << "    const char* override_name,\n";
    stream << "    const char* override_value,\n";
    stream << "    const char* argument) {\n";
    stream << "    if (executable_path == nullptr || output_path == nullptr || working_directory == nullptr ||\n";
    stream << "        override_name == nullptr || override_value == nullptr || argument == nullptr) {\n";
    stream << "        return -1;\n";
    stream << "    }\n";
    stream << "    if (std::string(argument) == \"--copperfin-verify-runtime-host\") {\n";
#if defined(_WIN32)
    stream << "        HANDLE verified_host = INVALID_HANDLE_VALUE;\n";
    stream << "        const bool verified = copperfin_runtime_bridge_read_verified_host(\n";
    stream << "            std::filesystem::path(executable_path), std::filesystem::path(output_path), verified_host);\n";
    stream << "        if (verified_host != INVALID_HANDLE_VALUE) {\n";
    stream << "            CloseHandle(verified_host);\n";
    stream << "        }\n";
#else
    stream << "        int verified_host = -1;\n";
    stream << "        const bool verified = copperfin_runtime_bridge_read_verified_host(\n";
    stream << "            std::filesystem::path(executable_path), std::filesystem::path(output_path), verified_host);\n";
    stream << "        if (verified_host >= 0) {\n";
    stream << "            close(verified_host);\n";
    stream << "        }\n";
#endif
    stream << "        return verified ? 0 : -1;\n";
    stream << "    }\n";
    stream << "    std::vector<std::string> arguments;\n";
    stream << "    if (*argument != '\\0') {\n";
    stream << "        arguments.emplace_back(argument);\n";
    stream << "    }\n";
    stream << "    std::vector<CopperfinRuntimeBridgeEnvironmentVariable> overrides;\n";
    stream << "    if (*override_name != '\\0') {\n";
    stream << "        overrides.push_back({override_name, override_value});\n";
    stream << "    }\n";
    stream << "    const CopperfinRuntimeBridgeDispatchExecution dispatch_execution{\n";
    stream << "        std::filesystem::path(executable_path),\n";
    stream << "        {},\n";
    stream << "        std::move(arguments),\n";
    stream << "        std::filesystem::path(working_directory),\n";
    stream << "        std::move(overrides),\n";
    stream << "        std::filesystem::path(output_path),\n";
    stream << "        {},\n";
    stream << "        true,\n";
    stream << "        false,\n";
    stream << "        0,\n";
    stream << "        false};\n";
    stream << "    const auto launch = copperfin_runtime_bridge_launch_process(dispatch_execution);\n";
    stream << "    return launch.launch_succeeded ? launch.exit_code : -1;\n";
    stream << "}\n";
    stream << "#endif\n";
    stream << "static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_launch_process(\n";
    stream << "    const CopperfinRuntimeBridgeDispatchExecution& dispatch_execution) {\n";
    stream << "    const bool launch_attempted = !dispatch_execution.executable_path.empty();\n";
    stream << "    int exit_code = -1;\n";
    stream << "    bool process_created = false;\n";
    stream << "#if defined(_WIN32)\n";
    stream << "    HANDLE verified_runtime_host = INVALID_HANDLE_VALUE;\n";
    stream << "    const bool runtime_host_verified = !dispatch_execution.verify_runtime_host ||\n";
    stream << "        copperfin_runtime_bridge_read_verified_host(\n";
    stream << "            dispatch_execution.executable_path,\n";
    stream << "            dispatch_execution.manifest_path,\n";
    stream << "            verified_runtime_host);\n";
    stream << "    const auto executable = copperfin_runtime_bridge_utf8_to_wide(copperfin_runtime_bridge_path_to_utf8_string(dispatch_execution.executable_path));\n";
    stream << "    std::wstring command_line = copperfin_runtime_bridge_quote_windows_argument(executable);\n";
    stream << "    for (const auto& argument : dispatch_execution.arguments) {\n";
    stream << "        command_line.push_back(L' ');\n";
    stream << "        command_line += copperfin_runtime_bridge_quote_windows_argument(copperfin_runtime_bridge_utf8_to_wide(argument));\n";
    stream << "    }\n";
    stream << "    std::vector<wchar_t> mutable_command_line(command_line.begin(), command_line.end());\n";
    stream << "    mutable_command_line.push_back(L'\\0');\n";
    stream << "    const auto environment_entries = copperfin_runtime_bridge_windows_environment(dispatch_execution.environment);\n";
    stream << "    std::vector<wchar_t> environment_block;\n";
    stream << "    for (const auto& entry : environment_entries) {\n";
    stream << "        environment_block.insert(environment_block.end(), entry.begin(), entry.end());\n";
    stream << "        environment_block.push_back(L'\\0');\n";
    stream << "    }\n";
    stream << "    environment_block.push_back(L'\\0');\n";
    stream << "    if (environment_entries.empty()) {\n";
    stream << "        environment_block.push_back(L'\\0');\n";
    stream << "    }\n";
    stream << "    SECURITY_ATTRIBUTES security_attributes{};\n";
    stream << "    security_attributes.nLength = sizeof(security_attributes);\n";
    stream << "    security_attributes.bInheritHandle = FALSE;\n";
    stream << "    const auto open_log = [&](const std::filesystem::path& path, bool capture) {\n";
    stream << "        if (!capture || path.empty()) {\n";
    stream << "            return static_cast<HANDLE>(nullptr);\n";
    stream << "        }\n";
    stream << "        return CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &security_attributes, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);\n";
    stream << "    };\n";
    stream << "    HANDLE stdout_handle = open_log(dispatch_execution.stdout_log_path, dispatch_execution.capture_stdout);\n";
    stream << "    HANDLE stderr_handle = open_log(dispatch_execution.stderr_log_path, dispatch_execution.capture_stderr);\n";
    stream << "    const bool log_handles_valid =\n";
    stream << "        (!dispatch_execution.capture_stdout || dispatch_execution.stdout_log_path.empty() || stdout_handle != INVALID_HANDLE_VALUE) &&\n";
    stream << "        (!dispatch_execution.capture_stderr || dispatch_execution.stderr_log_path.empty() || stderr_handle != INVALID_HANDLE_VALUE);\n";
    stream << "    if (runtime_host_verified && log_handles_valid && !executable.empty()) {\n";
    stream << "        const auto is_real_handle = [](HANDLE handle) {\n";
    stream << "            return handle != nullptr && handle != INVALID_HANDLE_VALUE;\n";
    stream << "        };\n";
    stream << "        const HANDLE standard_input_handle = GetStdHandle(STD_INPUT_HANDLE);\n";
    stream << "        const HANDLE standard_output_handle = stdout_handle == nullptr ? GetStdHandle(STD_OUTPUT_HANDLE) : stdout_handle;\n";
    stream << "        const HANDLE standard_error_handle = stderr_handle == nullptr ? GetStdHandle(STD_ERROR_HANDLE) : stderr_handle;\n";
    stream << "        std::vector<HANDLE> duplicated_standard_handles;\n";
    stream << "        const auto duplicate_for_child = [&](HANDLE source) {\n";
    stream << "            if (!is_real_handle(source)) {\n";
    stream << "                return static_cast<HANDLE>(nullptr);\n";
    stream << "            }\n";
    stream << "            HANDLE duplicate = nullptr;\n";
    stream << "            if (!DuplicateHandle(GetCurrentProcess(), source, GetCurrentProcess(), &duplicate, 0, TRUE, DUPLICATE_SAME_ACCESS) ||\n";
    stream << "                SetHandleInformation(duplicate, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT) == FALSE) {\n";
    stream << "                if (duplicate != nullptr) {\n";
    stream << "                    CloseHandle(duplicate);\n";
    stream << "                }\n";
    stream << "                return static_cast<HANDLE>(nullptr);\n";
    stream << "            }\n";
    stream << "            duplicated_standard_handles.push_back(duplicate);\n";
    stream << "            return duplicate;\n";
    stream << "        };\n";
    stream << "        const HANDLE child_input_handle = duplicate_for_child(standard_input_handle);\n";
    stream << "        const HANDLE child_output_handle = duplicate_for_child(standard_output_handle);\n";
    stream << "        const HANDLE child_error_handle = duplicate_for_child(standard_error_handle);\n";
    stream << "        const bool standard_handles_ready =\n";
    stream << "            (!is_real_handle(standard_input_handle) || child_input_handle != nullptr) &&\n";
    stream << "            (!is_real_handle(standard_output_handle) || child_output_handle != nullptr) &&\n";
    stream << "            (!is_real_handle(standard_error_handle) || child_error_handle != nullptr);\n";
    stream << "        std::vector<HANDLE> inherited_handles;\n";
    stream << "        if (child_input_handle != nullptr) {\n";
    stream << "            inherited_handles.push_back(child_input_handle);\n";
    stream << "        }\n";
    stream << "        if (child_output_handle != nullptr) {\n";
    stream << "            inherited_handles.push_back(child_output_handle);\n";
    stream << "        }\n";
    stream << "        if (child_error_handle != nullptr) {\n";
    stream << "            inherited_handles.push_back(child_error_handle);\n";
    stream << "        }\n";
    stream << "        SIZE_T attribute_list_size = 0U;\n";
    stream << "        std::vector<unsigned char> attribute_storage;\n";
    stream << "        PPROC_THREAD_ATTRIBUTE_LIST attribute_list = nullptr;\n";
    stream << "        bool attribute_list_initialized = false;\n";
    stream << "        bool restricted_handle_list_ready = inherited_handles.empty();\n";
    stream << "        if (!inherited_handles.empty()) {\n";
    stream << "            InitializeProcThreadAttributeList(nullptr, 1U, 0U, &attribute_list_size);\n";
    stream << "            attribute_storage.resize(attribute_list_size);\n";
    stream << "            attribute_list = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(attribute_storage.data());\n";
    stream << "            attribute_list_initialized = attribute_list_size > 0U &&\n";
    stream << "                InitializeProcThreadAttributeList(attribute_list, 1U, 0U, &attribute_list_size) != FALSE;\n";
    stream << "            restricted_handle_list_ready = attribute_list_initialized &&\n";
    stream << "                UpdateProcThreadAttribute(attribute_list, 0U, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,\n";
    stream << "                    inherited_handles.data(), inherited_handles.size() * sizeof(HANDLE), nullptr, nullptr) != FALSE;\n";
    stream << "        }\n";
    stream << "        STARTUPINFOEXW startup_info{};\n";
    stream << "        startup_info.StartupInfo.cb = restricted_handle_list_ready && !inherited_handles.empty()\n";
    stream << "            ? sizeof(startup_info)\n";
    stream << "            : sizeof(startup_info.StartupInfo);\n";
    stream << "        startup_info.StartupInfo.dwFlags = STARTF_USESTDHANDLES;\n";
    stream << "        startup_info.StartupInfo.hStdInput = child_input_handle;\n";
    stream << "        startup_info.StartupInfo.hStdOutput = child_output_handle;\n";
    stream << "        startup_info.StartupInfo.hStdError = child_error_handle;\n";
    stream << "        if (restricted_handle_list_ready && !inherited_handles.empty()) {\n";
    stream << "            startup_info.lpAttributeList = attribute_list;\n";
    stream << "        }\n";
    stream << "        PROCESS_INFORMATION process_info{};\n";
    stream << "        const auto working_directory = dispatch_execution.working_directory.empty()\n";
    stream << "            ? std::wstring{}\n";
    stream << "            : dispatch_execution.working_directory.wstring();\n";
    stream << "        const DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT |\n";
    stream << "            (restricted_handle_list_ready && !inherited_handles.empty() ? EXTENDED_STARTUPINFO_PRESENT : 0U);\n";
    stream << "        process_created = standard_handles_ready && restricted_handle_list_ready && CreateProcessW(\n";
    stream << "            nullptr,\n";
    stream << "            mutable_command_line.data(),\n";
    stream << "            nullptr,\n";
    stream << "            nullptr,\n";
    stream << "            !inherited_handles.empty(),\n";
    stream << "            creation_flags,\n";
    stream << "            environment_block.data(),\n";
    stream << "            working_directory.empty() ? nullptr : working_directory.c_str(),\n";
    stream << "            &startup_info.StartupInfo,\n";
    stream << "            &process_info) != FALSE;\n";
    stream << "        if (attribute_list_initialized) {\n";
    stream << "            DeleteProcThreadAttributeList(attribute_list);\n";
    stream << "        }\n";
    stream << "        if (process_created) {\n";
    stream << "            WaitForSingleObject(process_info.hProcess, INFINITE);\n";
    stream << "            DWORD process_exit_code = static_cast<DWORD>(-1);\n";
    stream << "            GetExitCodeProcess(process_info.hProcess, &process_exit_code);\n";
    stream << "            exit_code = static_cast<int>(process_exit_code);\n";
    stream << "            CloseHandle(process_info.hThread);\n";
    stream << "            CloseHandle(process_info.hProcess);\n";
    stream << "        }\n";
    stream << "        for (const HANDLE handle : duplicated_standard_handles) {\n";
    stream << "            CloseHandle(handle);\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    if (stdout_handle != nullptr && stdout_handle != INVALID_HANDLE_VALUE) {\n";
    stream << "        CloseHandle(stdout_handle);\n";
    stream << "    }\n";
    stream << "    if (stderr_handle != nullptr && stderr_handle != INVALID_HANDLE_VALUE) {\n";
    stream << "        CloseHandle(stderr_handle);\n";
    stream << "    }\n";
    stream << "    if (verified_runtime_host != INVALID_HANDLE_VALUE) {\n";
    stream << "        CloseHandle(verified_runtime_host);\n";
    stream << "    }\n";
    stream << "#else\n";
    stream << "    int verified_runtime_host = -1;\n";
    stream << "    const bool runtime_host_verified = !dispatch_execution.verify_runtime_host ||\n";
    stream << "        copperfin_runtime_bridge_read_verified_host(\n";
    stream << "            dispatch_execution.executable_path,\n";
    stream << "            dispatch_execution.manifest_path,\n";
    stream << "            verified_runtime_host);\n";
    stream << "    std::vector<std::string> argument_values;\n";
    stream << "    argument_values.reserve(dispatch_execution.arguments.size() + 1U);\n";
    stream << "    argument_values.push_back(copperfin_runtime_bridge_path_to_utf8_string(dispatch_execution.executable_path));\n";
    stream << "    argument_values.insert(argument_values.end(), dispatch_execution.arguments.begin(), dispatch_execution.arguments.end());\n";
    stream << "    std::vector<char*> argument_values_pointers;\n";
    stream << "    argument_values_pointers.reserve(argument_values.size() + 1U);\n";
    stream << "    for (auto& argument : argument_values) {\n";
    stream << "        argument_values_pointers.push_back(argument.data());\n";
    stream << "    }\n";
    stream << "    argument_values_pointers.push_back(nullptr);\n";
    stream << "    const auto environment_values = copperfin_runtime_bridge_posix_environment(dispatch_execution.environment);\n";
    stream << "    std::vector<char*> environment_value_pointers;\n";
    stream << "    environment_value_pointers.reserve(environment_values.size() + 1U);\n";
    stream << "    for (const auto& environment_value : environment_values) {\n";
    stream << "        environment_value_pointers.push_back(const_cast<char*>(environment_value.c_str()));\n";
    stream << "    }\n";
    stream << "    environment_value_pointers.push_back(nullptr);\n";
    stream << "    const pid_t child_process = runtime_host_verified ? fork() : static_cast<pid_t>(-1);\n";
    stream << "    if (child_process == 0) {\n";
    stream << "        if (!dispatch_execution.working_directory.empty() && chdir(dispatch_execution.working_directory.c_str()) != 0) {\n";
    stream << "            _exit(126);\n";
    stream << "        }\n";
    stream << "        const auto redirect_output = [&](const std::filesystem::path& path, bool capture, int descriptor) {\n";
    stream << "            if (!capture || path.empty()) {\n";
    stream << "                return true;\n";
    stream << "            }\n";
    stream << "            const int file_descriptor = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);\n";
    stream << "            if (file_descriptor < 0 || dup2(file_descriptor, descriptor) < 0) {\n";
    stream << "                if (file_descriptor >= 0) {\n";
    stream << "                    close(file_descriptor);\n";
    stream << "                }\n";
    stream << "                return false;\n";
    stream << "            }\n";
    stream << "            close(file_descriptor);\n";
    stream << "            return true;\n";
    stream << "        };\n";
    stream << "        if (!redirect_output(dispatch_execution.stdout_log_path, dispatch_execution.capture_stdout, STDOUT_FILENO) ||\n";
    stream << "            !redirect_output(dispatch_execution.stderr_log_path, dispatch_execution.capture_stderr, STDERR_FILENO)) {\n";
    stream << "            _exit(126);\n";
    stream << "        }\n";
    stream << "#if defined(__linux__)\n";
    stream << "        if (dispatch_execution.verify_runtime_host) {\n";
    stream << "            fexecve(verified_runtime_host, argument_values_pointers.data(), environment_value_pointers.data());\n";
    stream << "        } else {\n";
    stream << "            execve(argument_values_pointers[0], argument_values_pointers.data(), environment_value_pointers.data());\n";
    stream << "        }\n";
    stream << "#else\n";
    stream << "        execve(argument_values_pointers[0], argument_values_pointers.data(), environment_value_pointers.data());\n";
    stream << "#endif\n";
    stream << "        _exit(127);\n";
    stream << "    }\n";
    stream << "    if (verified_runtime_host >= 0) {\n";
    stream << "        close(verified_runtime_host);\n";
    stream << "    }\n";
    stream << "    if (child_process > 0) {\n";
    stream << "        int child_status = 0;\n";
    stream << "        pid_t waited_process = 0;\n";
    stream << "        do {\n";
    stream << "            waited_process = waitpid(child_process, &child_status, 0);\n";
    stream << "        } while (waited_process < 0 && errno == EINTR);\n";
    stream << "        if (waited_process == child_process && WIFEXITED(child_status)) {\n";
    stream << "            exit_code = WEXITSTATUS(child_status);\n";
    stream << "            process_created = true;\n";
    stream << "        } else if (waited_process == child_process && WIFSIGNALED(child_status)) {\n";
    stream << "            exit_code = 128 + WTERMSIG(child_status);\n";
    stream << "            process_created = true;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "#endif\n";
    stream << "    const bool launch_succeeded = launch_attempted && process_created && exit_code == dispatch_execution.expected_exit_code;\n";
    stream << "    return CopperfinRuntimeBridgeProcessLaunch{\n";
    stream << "        dispatch_execution.executable_path,\n";
    stream << "        dispatch_execution.arguments,\n";
    stream << "        dispatch_execution.working_directory,\n";
    stream << "        dispatch_execution.environment,\n";
    stream << "        dispatch_execution.stdout_log_path,\n";
    stream << "        dispatch_execution.stderr_log_path,\n";
    stream << "        dispatch_execution.capture_stdout,\n";
    stream << "        dispatch_execution.capture_stderr,\n";
    stream << "        launch_attempted,\n";
    stream << "        launch_succeeded,\n";
    stream << "        exit_code,\n";
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
    stream << "    const CopperfinRuntimeBridgeResponseReadPlan& response_read_plan,\n";
    stream << "    const std::string& response_document) {\n";
    stream << "    const auto& failure_policy_plan =\n";
    stream << "        response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan;\n";
    stream << "    const bool response_missing = response_read_plan.require_existing_response && response_document.empty();\n";
    stream << "    const bool should_use_fallback_return =\n";
    stream << "        host_failure.should_use_fallback_return || response_missing;\n";
    stream << "    return CopperfinRuntimeBridgeMissingResponseEvaluation{\n";
    stream << "        response_missing,\n";
    stream << "        should_use_fallback_return,\n";
    stream << "        should_use_fallback_return ? failure_policy_plan.diagnostics_fallback : host_failure.diagnostics_value,\n";
    stream << "        failure_policy_plan.fallback_return_value};\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_find_json_field_value_start(\n";
    stream << "    const std::string& response_document,\n";
    stream << "    const std::string& field_name,\n";
    stream << "    std::size_t& value_start) {\n";
    stream << "    const auto field_token = std::string(\"\\\"\") + field_name + \"\\\"\";\n";
    stream << "    value_start = std::string::npos;\n";
    stream << "    std::size_t object_depth = 0U;\n";
    stream << "    std::size_t array_depth = 0U;\n";
    stream << "    for (std::size_t index = 0U; index < response_document.size(); ++index) {\n";
    stream << "        const char ch = response_document[index];\n";
    stream << "        if (ch == '\"') {\n";
    stream << "            std::size_t string_end = index + 1U;\n";
    stream << "            bool escaping = false;\n";
    stream << "            for (; string_end < response_document.size(); ++string_end) {\n";
    stream << "                const char string_ch = response_document[string_end];\n";
    stream << "                if (escaping) {\n";
    stream << "                    escaping = false;\n";
    stream << "                    continue;\n";
    stream << "                }\n";
    stream << "                if (string_ch == '\\\\') {\n";
    stream << "                    escaping = true;\n";
    stream << "                    continue;\n";
    stream << "                }\n";
    stream << "                if (string_ch == '\"') {\n";
    stream << "                    break;\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            if (string_end >= response_document.size()) {\n";
    stream << "                return false;\n";
    stream << "            }\n";
    stream << "            if (object_depth == 1U && array_depth == 0U &&\n";
    stream << "                string_end + 1U == index + field_token.size() &&\n";
    stream << "                response_document.compare(index, field_token.size(), field_token) == 0) {\n";
    stream << "                const auto colon_offset = response_document.find_first_not_of(\" \\t\\r\\n\", string_end + 1U);\n";
    stream << "                if (colon_offset != std::string::npos && response_document[colon_offset] == ':') {\n";
    stream << "                    value_start = response_document.find_first_not_of(\" \\t\\r\\n\", colon_offset + 1U);\n";
    stream << "                    if (value_start == std::string::npos) {\n";
    stream << "                        return false;\n";
    stream << "                    }\n";
    stream << "                    return true;\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            index = string_end;\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        if (ch == '{') {\n";
    stream << "            ++object_depth;\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        if (ch == '}' && object_depth > 0U) {\n";
    stream << "            --object_depth;\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        if (ch == '[') {\n";
    stream << "            ++array_depth;\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        if (ch == ']' && array_depth > 0U) {\n";
    stream << "            --array_depth;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    return false;\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_response_document_has_field(\n";
    stream << "    const std::string& response_document,\n";
    stream << "    const std::string& field_name) {\n";
    stream << "    std::size_t value_start = std::string::npos;\n";
    stream << "    return copperfin_runtime_bridge_find_json_field_value_start(response_document, field_name, value_start);\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_response_document_has_required_fields(\n";
    stream << "    const std::string& response_document,\n";
    stream << "    const std::vector<std::string>& required_response_fields) {\n";
    stream << "    for (const auto& required_response_field : required_response_fields) {\n";
    stream << "        if (!copperfin_runtime_bridge_response_document_has_field(\n";
    stream << "                response_document,\n";
    stream << "                required_response_field)) {\n";
    stream << "            return false;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_schema_version_field_name();\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_media_type_field_name();\n\n";
    stream << "static std::string copperfin_runtime_bridge_extract_json_field(\n";
    stream << "    const std::string& response_document,\n";
    stream << "    const std::string& field_name);\n\n";
    stream << "static CopperfinRuntimeBridgeResponseValidationEvaluation copperfin_runtime_bridge_evaluate_response_validation(\n";
    stream << "    const CopperfinRuntimeBridgeMissingResponseEvaluation& missing_response,\n";
    stream << "    const CopperfinRuntimeBridgeResponseValidationPlan& response_validation_plan,\n";
    stream << "    const std::string& response_document) {\n";
    stream << "    const bool response_document_available = !response_document.empty();\n";
    stream << "    const bool required_response_fields_present =\n";
    stream << "        copperfin_runtime_bridge_response_document_has_required_fields(\n";
    stream << "            response_document,\n";
    stream << "            response_validation_plan.required_response_fields);\n";
    stream << "    const auto response_media_type = copperfin_runtime_bridge_extract_json_field(\n";
    stream << "        response_document,\n";
    stream << "        copperfin_build_runtime_bridge_response_media_type_field_name());\n";
    stream << "    const bool response_media_type_matches =\n";
    stream << "        response_media_type == response_validation_plan.expected_response_media_type;\n";
    stream << "    const auto response_schema_version = copperfin_runtime_bridge_extract_json_field(\n";
    stream << "        response_document,\n";
    stream << "        copperfin_build_runtime_bridge_schema_version_field_name());\n";
    stream << "    const bool response_schema_version_matches =\n";
    stream << "        response_schema_version == response_validation_plan.expected_schema_version;\n";
    stream << "    const bool validation_failed =\n";
    stream << "        missing_response.should_use_fallback_return || !response_document_available || !required_response_fields_present || !response_media_type_matches || !response_schema_version_matches;\n";
    stream << "    const bool should_use_fallback_return = validation_failed;\n";
    stream << "    return CopperfinRuntimeBridgeResponseValidationEvaluation{\n";
    stream << "        validation_failed,\n";
    stream << "        response_document_available,\n";
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
    stream << "         copperfin_build_runtime_bridge_routine_kind_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_source_path_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_source_line_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_parameter_declaration_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_parameter_names_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_parameter_count_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_schema_version_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_parameters_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_request_media_type_field_name()},\n";
    stream << "        {copperfin_build_runtime_bridge_status_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_return_value_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_response_media_type_field_name(),\n";
    stream << "         copperfin_build_runtime_bridge_schema_version_field_name(),\n";
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
    stream << "        std::error_code parent_directory_error;\n";
    stream << "        std::filesystem::create_directories(plan.target_path.parent_path(), parent_directory_error);\n";
    stream << "        if (parent_directory_error) {\n";
    stream << "            return false;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    const auto remove_artifact = [](const std::filesystem::path& path) {\n";
    stream << "        if (path.empty()) {\n";
    stream << "            return true;\n";
    stream << "        }\n";
    stream << "        std::error_code error;\n";
    stream << "        std::filesystem::remove(path, error);\n";
    stream << "        return !error;\n";
    stream << "    };\n";
    stream << "    const auto& transport_plan =\n";
    stream << "        plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan;\n";
    stream << "    const auto& observation_plan =\n";
    stream << "        transport_plan.execution_plan.observation_plan;\n";
    stream << "    if (!remove_artifact(plan.target_path) ||\n";
    stream << "        !remove_artifact(transport_plan.response_path) ||\n";
    stream << "        !remove_artifact(observation_plan.stdout_log_path) ||\n";
    stream << "        !remove_artifact(observation_plan.stderr_log_path)) {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    std::ofstream out(plan.target_path);\n";
    stream << "    if (!out) {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    out << plan.request_artifact.request_document;\n";
    stream << "    return out.good();\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_failed_process_launch(\n";
    stream << "    const CopperfinRuntimeBridgeDispatchExecution& dispatch_execution) {\n";
    stream << "    return CopperfinRuntimeBridgeProcessLaunch{\n";
    stream << "        dispatch_execution.executable_path,\n";
    stream << "        dispatch_execution.arguments,\n";
    stream << "        dispatch_execution.working_directory,\n";
    stream << "        dispatch_execution.environment,\n";
    stream << "        dispatch_execution.stdout_log_path,\n";
    stream << "        dispatch_execution.stderr_log_path,\n";
    stream << "        dispatch_execution.capture_stdout,\n";
    stream << "        dispatch_execution.capture_stderr,\n";
    stream << "        false,\n";
    stream << "        false,\n";
    stream << "        -1,\n";
    stream << "        dispatch_execution.expected_exit_code};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_read_mode() {\n";
    stream << "    return \"read_text\";\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_require_existing_response_policy() {\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeResponseReadPlan copperfin_build_runtime_bridge_response_read_plan(\n";
    stream << "    CopperfinRuntimeBridgeRequestWritePlan request_write_plan,\n";
    stream << "    bool request_write_succeeded) {\n";
    stream << "    const auto source_path =\n";
    stream << "        request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.response_path;\n";
    stream << "    const auto read_mode = copperfin_build_runtime_bridge_response_read_mode();\n";
    stream << "    return CopperfinRuntimeBridgeResponseReadPlan{\n";
    stream << "        std::move(request_write_plan),\n";
    stream << "        request_write_succeeded,\n";
    stream << "        source_path,\n";
    stream << "        read_mode,\n";
    stream << "        copperfin_runtime_bridge_require_existing_response_policy()};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_empty_response_document() {\n";
    stream << "    return \"\";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_execute_read_response(\n";
    stream << "    const CopperfinRuntimeBridgeResponseReadPlan& plan) {\n";
    stream << "    if (!plan.request_write_succeeded) {\n";
    stream << "        return copperfin_build_runtime_bridge_empty_response_document();\n";
    stream << "    }\n";
    stream << "    std::error_code response_exists_error;\n";
    stream << "    if (plan.require_existing_response &&\n";
    stream << "        (!std::filesystem::exists(plan.source_path, response_exists_error) || response_exists_error)) {\n";
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
    stream << "static void copperfin_runtime_bridge_cleanup_artifacts(\n";
    stream << "    const CopperfinRuntimeBridgeResponseReadPlan& plan) {\n";
    stream << "    const auto remove_artifact = [](const std::filesystem::path& path) {\n";
    stream << "        if (path.empty()) {\n";
    stream << "            return;\n";
    stream << "        }\n";
    stream << "        std::error_code ignored;\n";
    stream << "        std::filesystem::remove(path, ignored);\n";
    stream << "    };\n";
    stream << "    const auto& transport_plan =\n";
    stream << "        plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan;\n";
    stream << "    const auto& observation_plan =\n";
    stream << "        transport_plan.execution_plan.observation_plan;\n";
    stream << "    remove_artifact(transport_plan.request_path);\n";
    stream << "    remove_artifact(transport_plan.response_path);\n";
    stream << "    remove_artifact(observation_plan.stdout_log_path);\n";
    stream << "    remove_artifact(observation_plan.stderr_log_path);\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeResponseArtifact copperfin_build_runtime_bridge_response_artifact(\n";
    stream << "    CopperfinRuntimeBridgeResponseReadPlan response_read_plan,\n";
    stream << "    std::string response_document) {\n";
    stream << "    return CopperfinRuntimeBridgeResponseArtifact{\n";
    stream << "        std::move(response_read_plan),\n";
    stream << "        std::move(response_document)};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_response_parse_kind() {\n";
    stream << "    return \"json_field_map\";\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_parse_json_string_at(\n";
    stream << "    const std::string& document,\n";
    stream << "    std::size_t value_start,\n";
    stream << "    std::size_t& value_end,\n";
    stream << "    std::string& decoded_value) {\n";
    stream << "    if (value_start >= document.size() || document[value_start] != '\"') {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    decoded_value.clear();\n";
    stream << "    for (std::size_t index = value_start + 1; index < document.size(); ++index) {\n";
    stream << "        const char ch = document[index];\n";
    stream << "        if (ch == '\\\\' && index + 1 < document.size()) {\n";
    stream << "            const char escaped = document[++index];\n";
    stream << "            switch (escaped) {\n";
    stream << "            case 'n': decoded_value.push_back('\\n'); break;\n";
    stream << "            case 'r': decoded_value.push_back('\\r'); break;\n";
    stream << "            case 't': decoded_value.push_back('\\t'); break;\n";
    stream << "            default: decoded_value.push_back(escaped); break;\n";
    stream << "            }\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        if (ch == '\"') {\n";
    stream << "            value_end = index + 1;\n";
    stream << "            return true;\n";
    stream << "        }\n";
    stream << "        decoded_value.push_back(ch);\n";
    stream << "    }\n";
    stream << "    return false;\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_runtime_bridge_extract_json_field(\n";
    stream << "    const std::string& response_document,\n";
    stream << "    const std::string& field_name) {\n";
    stream << "    std::size_t value_start = std::string::npos;\n";
    stream << "    if (!copperfin_runtime_bridge_find_json_field_value_start(response_document, field_name, value_start)) {\n";
    stream << "        return \"\";\n";
    stream << "    }\n";
    stream << "    if (response_document[value_start] == '\"') {\n";
    stream << "        std::size_t string_end = value_start;\n";
    stream << "        std::string decoded_value;\n";
    stream << "        return copperfin_runtime_bridge_parse_json_string_at(response_document, value_start, string_end, decoded_value)\n";
    stream << "            ? decoded_value\n";
    stream << "            : std::string{};\n";
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
    stream << "    CopperfinRuntimeBridgeResponseParsePlan response_parse_plan,\n";
    stream << "    CopperfinRuntimeBridgeParsedResponse parsed_response) {\n";
    stream << "    const auto& response_validation_plan =\n";
    stream << "        response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan;\n";
    stream << "    const auto& failure_policy_plan = response_validation_plan.failure_policy_plan;\n";
    stream << "    return CopperfinRuntimeBridgeInterpretedResultPlan{\n";
    stream << "        std::move(response_parse_plan),\n";
    stream << "        std::move(parsed_response),\n";
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
    stream << "    const auto& parsed_response = plan.parsed_response;\n";
    stream << "    const bool matched_success_status = parsed_response.status_value == plan.success_status_value;\n";
    stream << "    const auto selected_return_value_representation = matched_success_status\n";
    stream << "        ? parsed_response.return_value_representation\n";
    stream << "        : plan.fallback_return_value;\n";
    stream << "    return CopperfinRuntimeBridgeInterpretedResult{\n";
    stream << "        matched_success_status,\n";
    stream << "        std::move(selected_return_value_representation),\n";
    stream << "        parsed_response.diagnostics_value,\n";
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
    stream << "    CopperfinRuntimeBridgeInterpretedResultPlan interpreted_result_plan,\n";
    stream << "    CopperfinRuntimeBridgeInterpretedResult interpreted_result) {\n";
    stream << "    const auto success_value_representation = interpreted_result.selected_return_value_representation;\n";
    stream << "    const int success_int_value = copperfin_parse_runtime_bridge_int_value_representation(\n";
    stream << "        success_value_representation);\n";
    stream << "    const auto fallback_value_representation = interpreted_result_plan.fallback_return_value;\n";
    stream << "    const int fallback_int_value = copperfin_parse_runtime_bridge_int_value_representation(\n";
    stream << "        fallback_value_representation);\n";
    stream << "    return CopperfinRuntimeBridgeNativeReturnPlan{\n";
    stream << "        std::move(interpreted_result_plan),\n";
    stream << "        std::move(interpreted_result),\n";
    stream << "        success_value_representation,\n";
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
    stream << "    const auto& interpreted_result = plan.interpreted_result;\n";
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
    stream << "        interpreted_result.diagnostics_value,\n";
    stream << "        interpreted_result.wrapper_return_surface};\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_success_comparator_token() {\n";
    stream << "    return \" == \";\n";
    stream << "}\n\n";
    stream << "static std::string copperfin_build_runtime_bridge_fallback_comparator_token() {\n";
    stream << "    return \" != \";\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeOutcomeSelectionPlan copperfin_build_runtime_bridge_outcome_selection_plan(\n";
    stream << "    CopperfinRuntimeBridgeNativeReturnPlan native_return_plan,\n";
    stream << "    CopperfinRuntimeBridgeNativeReturn native_return) {\n";
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
    stream << "        std::move(native_return),\n";
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
    stream << "    const auto& native_return = plan.native_return;\n";
    stream << "    const auto selected_condition = native_return.matched_success_status\n";
    stream << "        ? plan.success_condition\n";
    stream << "        : plan.fallback_condition;\n";
    stream << "    return CopperfinRuntimeBridgeOutcomeSelection{\n";
    stream << "        native_return.matched_success_status,\n";
    stream << "        std::move(selected_condition),\n";
    stream << "        native_return.diagnostics_value,\n";
    stream << "        native_return.selected_value_representation,\n";
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
    stream << "    CopperfinRuntimeBridgeOutcomeSelectionPlan outcome_selection_plan,\n";
    stream << "    CopperfinRuntimeBridgeOutcomeSelection outcome_selection) {\n";
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
    stream << "        std::move(outcome_selection),\n";
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
    stream << "    const auto& outcome_selection = plan.outcome_selection;\n";
    stream << "    return CopperfinRuntimeBridgeReturnMaterialization{\n";
    stream << "        outcome_selection.matched_success_status,\n";
    stream << "        plan.outcome_selection_plan.success_condition,\n";
    stream << "        outcome_selection.selected_condition,\n";
    stream << "        outcome_selection.diagnostics_value,\n";
    stream << "        plan.success_return_statement,\n";
    stream << "        plan.fallback_return_statement,\n";
    stream << "        outcome_selection.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnEmissionPlan copperfin_build_runtime_bridge_return_emission_plan(\n";
    stream << "    CopperfinRuntimeBridgeReturnMaterializationPlan return_materialization_plan,\n";
    stream << "    CopperfinRuntimeBridgeReturnMaterialization return_materialization) {\n";
    stream << "    const auto success_branch_statement =\n";
    stream << "        \"if (\" + return_materialization.success_condition + \") { \"\n";
    stream << "        + return_materialization.success_return_statement + \" }\";\n";
    stream << "    const auto fallback_branch_statement =\n";
    stream << "        \"else { \" + return_materialization.fallback_return_statement + \" }\";\n";
    stream << "    const auto emitted_return_block =\n";
    stream << "        success_branch_statement + \" \" + fallback_branch_statement;\n";
    stream << "    return CopperfinRuntimeBridgeReturnEmissionPlan{\n";
    stream << "        std::move(return_materialization_plan),\n";
    stream << "        std::move(return_materialization),\n";
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
    stream << "    const auto& return_materialization = plan.return_materialization;\n";
    stream << "    return CopperfinRuntimeBridgeReturnEmission{\n";
    stream << "        return_materialization.matched_success_status,\n";
    stream << "        return_materialization.selected_condition,\n";
    stream << "        return_materialization.diagnostics_value,\n";
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
    stream << "    CopperfinRuntimeBridgeReturnEmission return_emission,\n";
    stream << "    std::string placeholder_return_statement) {\n";
    stream << "    const auto adopted_return_block = return_emission.emitted_return_block;\n";
    stream << "    return CopperfinRuntimeBridgeFinalReturnAdoptionPlan{\n";
    stream << "        std::move(return_emission_plan),\n";
    stream << "        std::move(return_emission),\n";
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
    stream << "    const auto& return_emission = plan.return_emission;\n";
    stream << "    return CopperfinRuntimeBridgeFinalReturnAdoption{\n";
    stream << "        return_emission.matched_success_status,\n";
    stream << "        return_emission.selected_condition,\n";
    stream << "        return_emission.diagnostics_value,\n";
    stream << "        plan.placeholder_return_statement,\n";
    stream << "        plan.adopted_return_block,\n";
    stream << "        plan.adoption_mode,\n";
    stream << "        return_emission.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static bool copperfin_runtime_bridge_activates_adopted_return_policy() {\n";
    stream << "    return false;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeReturnActivationPlan copperfin_build_runtime_bridge_return_activation_plan(\n";
    stream << "    CopperfinRuntimeBridgeFinalReturnAdoptionPlan final_return_adoption_plan,\n";
    stream << "    CopperfinRuntimeBridgeFinalReturnAdoption final_return_adoption) {\n";
    stream << "    const auto active_return_block = final_return_adoption.adopted_return_block;\n";
    stream << "    return CopperfinRuntimeBridgeReturnActivationPlan{\n";
    stream << "        std::move(final_return_adoption_plan),\n";
    stream << "        std::move(final_return_adoption),\n";
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
    stream << "    const auto& final_return_adoption = plan.final_return_adoption;\n";
    stream << "    return CopperfinRuntimeBridgeReturnActivation{\n";
    stream << "        final_return_adoption.matched_success_status,\n";
    stream << "        final_return_adoption.selected_condition,\n";
    stream << "        final_return_adoption.diagnostics_value,\n";
    stream << "        plan.activates_adopted_return,\n";
    stream << "        plan.activation_mode,\n";
    stream << "        plan.active_return_block,\n";
    stream << "        final_return_adoption.native_return_surface};\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeStubReturnPlan copperfin_build_runtime_bridge_stub_return_plan(\n";
    stream << "    CopperfinRuntimeBridgeReturnActivationPlan return_activation_plan,\n";
    stream << "    CopperfinRuntimeBridgeReturnActivation return_activation) {\n";
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
    stream << "        std::move(return_activation),\n";
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
    stream << "    const auto& return_activation = plan.return_activation;\n";
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
    stream << "        return_activation.selected_condition,\n";
    stream << "        return_activation.diagnostics_value,\n";
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
    stream << "    CopperfinRuntimeBridgeStubReturn stub_return) {\n";
    stream << "    return CopperfinRuntimeBridgePlaceholderReturnValuePlan{\n";
    stream << "        std::move(stub_return_plan),\n";
    stream << "        std::move(stub_return),\n";
    stream << "        stub_return.emits_placeholder_return,\n";
    stream << "        stub_return.emitted_return_statement,\n";
    stream << "        stub_return.deferred_return_block,\n";
    stream << "        stub_return.activation_mode,\n";
    stream << "        stub_return.adoption_mode,\n";
    stream << "        stub_return.keeps_placeholder_return_active,\n";
    stream << "        stub_return.adopts_placeholder_replacement,\n";
    stream << "        stub_return.placeholder_fallback_int_value,\n";
    stream << "        stub_return.placeholder_fallback_value_representation};\n";
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
    stream << "    const auto& stub_return = plan.stub_return;\n";
    stream << "    return CopperfinRuntimeBridgePlaceholderReturnValue{\n";
    stream << "        stub_return.matched_success_status,\n";
    stream << "        stub_return.selected_condition,\n";
    stream << "        stub_return.diagnostics_value,\n";
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
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnIntAdmission& placeholder_return_int_admission,\n";
    stream << "    int placeholder_return_int) {\n";
    stream << "    return CopperfinRuntimeBridgeStubEmissionAdmission{\n";
    stream << "        placeholder_return_int_admission.should_emit_placeholder_return,\n";
    stream << "        placeholder_return_int_admission.diagnostics_value,\n";
    stream << "        placeholder_return_int};\n";
    stream << "}\n\n";
    stream << "static int copperfin_runtime_bridge_execute_placeholder_return_int(\n";
    stream << "    const CopperfinRuntimeBridgePlaceholderReturnIntAdmission& placeholder_return_int_admission) {\n";
    stream << "    if (placeholder_return_int_admission.should_return_int) {\n";
    stream << "        return placeholder_return_int_admission.selected_int_value;\n";
    stream << "    }\n";
    stream << "    return placeholder_return_int_admission.selected_int_value;\n";
    stream << "}\n\n";
    stream << "static CopperfinRuntimeBridgeStubEmission copperfin_runtime_bridge_execute_stub_emission(\n";
    stream << "    const CopperfinRuntimeBridgeStubEmissionAdmission& stub_emission_admission) {\n";
    stream << "    return CopperfinRuntimeBridgeStubEmission{\n";
    stream << "        stub_emission_admission.should_emit_stub_return,\n";
    stream << "        stub_emission_admission.diagnostics_value,\n";
    stream << "        stub_emission_admission.selected_int_value};\n";
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
            stream << "    const auto stub_emission_wrapper =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_wrapper(\n";
            stream << "            copperfin_build_runtime_bridge_fll_int_return_surface(),\n";
            stream << "            " << kFllDefaultReturnHelper << ");\n";
            stream << "    const auto descriptor = copperfin_build_runtime_bridge_descriptor(\""
                   << quote_manifest_value(symbol) << "\", \"" << quote_manifest_value(routine_kind)
                   << "\", \"" << quote_manifest_value(location.file_path) << "\", " << location.line
                   << "U, \"" << quote_manifest_value(parameter_declaration_kind) << "\", \""
                   << quote_manifest_value(parameter_name_manifest) << "\", " << parameter_count
                   << "U, reinterpret_cast<void*>(&" << symbol << "), stub_emission_wrapper);\n";
            stream << "    const auto invocation = copperfin_build_runtime_bridge_invocation(\n";
            stream << "        descriptor);\n";
            stream << "    const auto call = copperfin_build_runtime_bridge_call(\n";
            stream << "        invocation,\n";
            stream << "        {{\"parm\", std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(parm))), \"ParamBlk*\"}});\n";
            stream << "    const auto placeholder_return_binding =\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_binding(\n";
            stream << "            copperfin_build_runtime_bridge_fll_int_return_surface());\n";
            stream << "    const auto result = copperfin_build_runtime_bridge_result(\n";
            stream << "        call,\n";
            stream << "        placeholder_return_binding);\n";
            stream << "    const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(\n";
            stream << "        result);\n";
            stream << "    const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(\n";
            stream << "        launch_plan);\n";
            stream << "    const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(\n";
            stream << "        observation_plan);\n";
            stream << "    const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(\n";
            stream << "        execution_plan);\n";
            stream << "    const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(\n";
            stream << "        transport_plan);\n";
            stream << "    const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(\n";
            stream << "        serialization_plan);\n";
            stream << "    const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);\n";
            stream << "    const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(\n";
            stream << "        dispatch_plan);\n";
            stream << "    const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(\n";
            stream << "        payload_plan,\n";
            stream << "        copperfin_build_runtime_bridge_fll_int_return_surface());\n";
            stream << "    const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n";
            stream << "        interpretation_plan,\n";
            stream << "        placeholder_return_binding.value_representation);\n";
            stream << "    const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n";
            stream << "        failure_policy);\n";
            stream << "    const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n";
            stream << "        response_validation);\n";
            stream << "    const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n";
            stream << "        request_artifact);\n";
            stream << "    const auto request_write_execution =\n";
            stream << "        copperfin_runtime_bridge_execute_write_request(request_write_plan);\n";
            stream << "    const auto process_launch = request_write_execution\n";
            stream << "        ? copperfin_runtime_bridge_launch_process(dispatch_execution)\n";
            stream << "        : copperfin_runtime_bridge_failed_process_launch(dispatch_execution);\n";
            stream << "    const auto host_failure =\n";
            stream << "        copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);\n";
            stream << "    const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n";
            stream << "        request_write_plan,\n";
            stream << "        request_write_execution);\n";
            stream << "    const auto response_document =\n";
            stream << "        copperfin_runtime_bridge_execute_read_response(response_read_plan);\n";
            stream << "    copperfin_runtime_bridge_cleanup_artifacts(response_read_plan);\n";
            stream << "    const auto missing_response =\n";
            stream << "        copperfin_runtime_bridge_evaluate_missing_response(\n";
            stream << "            host_failure,\n";
            stream << "            response_read_plan,\n";
            stream << "            response_document);\n";
            stream << "    const auto response_validation_evaluation =\n";
            stream << "        copperfin_runtime_bridge_evaluate_response_validation(\n";
            stream << "            missing_response,\n";
            stream << "            response_validation,\n";
            stream << "            response_document);\n";
            stream << "    const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n";
            stream << "        response_read_plan,\n";
            stream << "        response_document);\n";
            stream << "    const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n";
            stream << "        response_artifact);\n";
            stream << "    const auto response_parse_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);\n";
            stream << "    const auto parsed_response =\n";
            stream << "        copperfin_runtime_bridge_execute_parse_response(response_parse_plan);\n";
            stream << "    const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n";
            stream << "        response_parse_plan,\n";
            stream << "        parsed_response);\n";
            stream << "    const auto interpreted_result_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);\n";
            stream << "    const auto interpreted_result =\n";
            stream << "        copperfin_runtime_bridge_execute_interpreted_result(interpreted_result_plan);\n";
            stream << "    const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n";
            stream << "        result,\n";
            stream << "        interpreted_result_plan,\n";
            stream << "        interpreted_result);\n";
            stream << "    const auto native_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);\n";
            stream << "    const auto native_return =\n";
            stream << "        copperfin_runtime_bridge_execute_native_return(native_return_plan);\n";
            stream << "    const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n";
            stream << "        native_return_plan,\n";
            stream << "        native_return);\n";
            stream << "    const auto outcome_selection_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);\n";
            stream << "    const auto outcome_selection =\n";
            stream << "        copperfin_runtime_bridge_execute_outcome_selection(outcome_selection_plan);\n";
            stream << "    const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n";
            stream << "        outcome_selection_plan,\n";
            stream << "        outcome_selection);\n";
            stream << "    const auto return_materialization_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);\n";
            stream << "    const auto return_materialization =\n";
            stream << "        copperfin_runtime_bridge_execute_return_materialization(return_materialization_plan);\n";
            stream << "    const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n";
            stream << "        return_materialization_plan,\n";
            stream << "        return_materialization);\n";
            stream << "    const auto return_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);\n";
            stream << "    const auto return_emission =\n";
            stream << "        copperfin_runtime_bridge_execute_return_emission(return_emission_plan);\n";
            stream << "    const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n";
            stream << "        return_emission_plan,\n";
            stream << "        return_emission,\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));\n";
            stream << "    const auto final_return_adoption_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);\n";
            stream << "    const auto final_return_adoption =\n";
            stream << "        copperfin_runtime_bridge_execute_final_return_adoption(final_return_adoption_plan);\n";
            stream << "    const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n";
            stream << "        final_return_adoption_plan,\n";
            stream << "        final_return_adoption);\n";
            stream << "    const auto return_activation_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);\n";
            stream << "    const auto return_activation =\n";
            stream << "        copperfin_runtime_bridge_execute_return_activation(return_activation_plan);\n";
            stream << "    const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n";
            stream << "        return_activation_plan,\n";
            stream << "        return_activation);\n";
            stream << "    const auto stub_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);\n";
            stream << "    const auto stub_return =\n";
            stream << "        copperfin_runtime_bridge_execute_stub_return(stub_return_plan);\n";
            stream << "    const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n";
            stream << "        stub_return_plan,\n";
            stream << "        stub_return);\n";
            stream << "    const auto placeholder_return_value_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_value =\n";
            stream << "        copperfin_runtime_bridge_execute_placeholder_return_value(placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_int_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_int =\n";
            stream << "        copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_int_admission);\n";
            stream << "    const auto stub_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission, placeholder_return_int);\n";
            stream << "    const auto stub_emission =\n";
            stream << "        copperfin_runtime_bridge_execute_stub_emission(stub_emission_admission);\n";
            stream << "    const auto stub_emission_return_surface =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_return_surface(\n";
            stream << "            stub_emission,\n";
            stream << "            placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.native_return_surface);\n";
            stream << "    return copperfin_runtime_bridge_apply_stub_emission_output(\n";
            stream << "        stub_emission_return_surface,\n";
            stream << "        placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter);\n";
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
            stream << "    const auto stub_emission_wrapper =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_wrapper(\n";
            stream << "            copperfin_build_runtime_bridge_native_int_return_surface(),\n";
            stream << "            copperfin_runtime_bridge_return_native_int);\n";
            stream << "    const auto descriptor = copperfin_build_runtime_bridge_descriptor(\""
                   << quote_manifest_value(symbol) << "\", \"" << quote_manifest_value(routine_kind)
                   << "\", \"" << quote_manifest_value(location.file_path) << "\", " << location.line
                   << "U, \"" << quote_manifest_value(parameter_declaration_kind) << "\", \""
                   << quote_manifest_value(parameter_name_manifest) << "\", " << parameter_count
                   << "U, reinterpret_cast<void*>(&" << symbol << "), stub_emission_wrapper);\n";
            stream << "    const auto invocation = copperfin_build_runtime_bridge_invocation(\n";
            stream << "        descriptor);\n";
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
            stream << "    const auto result = copperfin_build_runtime_bridge_result(\n";
            stream << "        call,\n";
            stream << "        placeholder_return_binding);\n";
            stream << "    const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(\n";
            stream << "        result);\n";
            stream << "    const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(\n";
            stream << "        launch_plan);\n";
            stream << "    const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(\n";
            stream << "        observation_plan);\n";
            stream << "    const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(\n";
            stream << "        execution_plan);\n";
            stream << "    const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(\n";
            stream << "        transport_plan);\n";
            stream << "    const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(\n";
            stream << "        serialization_plan);\n";
            stream << "    const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);\n";
            stream << "    const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(\n";
            stream << "        dispatch_plan);\n";
            stream << "    const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(\n";
            stream << "        payload_plan,\n";
            stream << "        copperfin_build_runtime_bridge_native_int_return_surface());\n";
            stream << "    const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n";
            stream << "        interpretation_plan,\n";
            stream << "        placeholder_return_binding.value_representation);\n";
            stream << "    const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n";
            stream << "        failure_policy);\n";
            stream << "    const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n";
            stream << "        response_validation);\n";
            stream << "    const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n";
            stream << "        request_artifact);\n";
            stream << "    const auto request_write_execution =\n";
            stream << "        copperfin_runtime_bridge_execute_write_request(request_write_plan);\n";
            stream << "    const auto process_launch = request_write_execution\n";
            stream << "        ? copperfin_runtime_bridge_launch_process(dispatch_execution)\n";
            stream << "        : copperfin_runtime_bridge_failed_process_launch(dispatch_execution);\n";
            stream << "    const auto host_failure =\n";
            stream << "        copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);\n";
            stream << "    const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n";
            stream << "        request_write_plan,\n";
            stream << "        request_write_execution);\n";
            stream << "    const auto response_document =\n";
            stream << "        copperfin_runtime_bridge_execute_read_response(response_read_plan);\n";
            stream << "    copperfin_runtime_bridge_cleanup_artifacts(response_read_plan);\n";
            stream << "    const auto missing_response =\n";
            stream << "        copperfin_runtime_bridge_evaluate_missing_response(\n";
            stream << "            host_failure,\n";
            stream << "            response_read_plan,\n";
            stream << "            response_document);\n";
            stream << "    const auto response_validation_evaluation =\n";
            stream << "        copperfin_runtime_bridge_evaluate_response_validation(\n";
            stream << "            missing_response,\n";
            stream << "            response_validation,\n";
            stream << "            response_document);\n";
            stream << "    const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n";
            stream << "        response_read_plan,\n";
            stream << "        response_document);\n";
            stream << "    const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n";
            stream << "        response_artifact);\n";
            stream << "    const auto response_parse_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);\n";
            stream << "    const auto parsed_response =\n";
            stream << "        copperfin_runtime_bridge_execute_parse_response(response_parse_plan);\n";
            stream << "    const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n";
            stream << "        response_parse_plan,\n";
            stream << "        parsed_response);\n";
            stream << "    const auto interpreted_result_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);\n";
            stream << "    const auto interpreted_result =\n";
            stream << "        copperfin_runtime_bridge_execute_interpreted_result(interpreted_result_plan);\n";
            stream << "    const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n";
            stream << "        result,\n";
            stream << "        interpreted_result_plan,\n";
            stream << "        interpreted_result);\n";
            stream << "    const auto native_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);\n";
            stream << "    const auto native_return =\n";
            stream << "        copperfin_runtime_bridge_execute_native_return(native_return_plan);\n";
            stream << "    const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n";
            stream << "        native_return_plan,\n";
            stream << "        native_return);\n";
            stream << "    const auto outcome_selection_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);\n";
            stream << "    const auto outcome_selection =\n";
            stream << "        copperfin_runtime_bridge_execute_outcome_selection(outcome_selection_plan);\n";
            stream << "    const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n";
            stream << "        outcome_selection_plan,\n";
            stream << "        outcome_selection);\n";
            stream << "    const auto return_materialization_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);\n";
            stream << "    const auto return_materialization =\n";
            stream << "        copperfin_runtime_bridge_execute_return_materialization(return_materialization_plan);\n";
            stream << "    const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n";
            stream << "        return_materialization_plan,\n";
            stream << "        return_materialization);\n";
            stream << "    const auto return_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);\n";
            stream << "    const auto return_emission =\n";
            stream << "        copperfin_runtime_bridge_execute_return_emission(return_emission_plan);\n";
            stream << "    const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n";
            stream << "        return_emission_plan,\n";
            stream << "        return_emission,\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));\n";
            stream << "    const auto final_return_adoption_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);\n";
            stream << "    const auto final_return_adoption =\n";
            stream << "        copperfin_runtime_bridge_execute_final_return_adoption(final_return_adoption_plan);\n";
            stream << "    const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n";
            stream << "        final_return_adoption_plan,\n";
            stream << "        final_return_adoption);\n";
            stream << "    const auto return_activation_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);\n";
            stream << "    const auto return_activation =\n";
            stream << "        copperfin_runtime_bridge_execute_return_activation(return_activation_plan);\n";
            stream << "    const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n";
            stream << "        return_activation_plan,\n";
            stream << "        return_activation);\n";
            stream << "    const auto stub_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);\n";
            stream << "    const auto stub_return =\n";
            stream << "        copperfin_runtime_bridge_execute_stub_return(stub_return_plan);\n";
            stream << "    const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n";
            stream << "        stub_return_plan,\n";
            stream << "        stub_return);\n";
            stream << "    const auto placeholder_return_value_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_value =\n";
            stream << "        copperfin_runtime_bridge_execute_placeholder_return_value(placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_int_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_int =\n";
            stream << "        copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_int_admission);\n";
            stream << "    const auto stub_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission, placeholder_return_int);\n";
            stream << "    const auto stub_emission =\n";
            stream << "        copperfin_runtime_bridge_execute_stub_emission(stub_emission_admission);\n";
            stream << "    const auto stub_emission_return_surface =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_return_surface(\n";
            stream << "            stub_emission,\n";
            stream << "            placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.native_return_surface);\n";
            stream << "    return copperfin_runtime_bridge_apply_stub_emission_output(\n";
            stream << "        stub_emission_return_surface,\n";
            stream << "        placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter);\n";
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
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.launcher_output_path).stem());
    const std::string output_extension =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.launcher_output_path).extension());
    const std::string output_directory = "..";
    const std::string wrapper_file_name =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.native_wrapper_source_path).filename());
    const std::string module_definition_file_name =
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(plan.module_definition_path).filename());

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
    stream << "foreach(COPPERFIN_CONFIGURATION IN LISTS CMAKE_CONFIGURATION_TYPES)\n";
    stream << "  string(TOUPPER \"${COPPERFIN_CONFIGURATION}\" COPPERFIN_CONFIGURATION_UPPER)\n";
    stream << "  set_target_properties(" << output_stem << " PROPERTIES\n";
    stream << "    \"LIBRARY_OUTPUT_DIRECTORY_${COPPERFIN_CONFIGURATION_UPPER}\" \"${CMAKE_CURRENT_SOURCE_DIR}/"
           << output_directory << "\"\n";
    stream << "    \"RUNTIME_OUTPUT_DIRECTORY_${COPPERFIN_CONFIGURATION_UPPER}\" \"${CMAKE_CURRENT_SOURCE_DIR}/"
           << output_directory << "\"\n";
    stream << "    \"ARCHIVE_OUTPUT_DIRECTORY_${COPPERFIN_CONFIGURATION_UPPER}\" \"${CMAKE_CURRENT_SOURCE_DIR}/"
           << output_directory << "\")\n";
    stream << "endforeach()\n";
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
    stream.imbue(std::locale::classic());
    const auto parameter_counts = collect_library_export_parameter_counts(plan);
    const auto parameter_names = collect_library_export_parameter_names(plan);
    const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
    const auto routine_kinds = collect_library_export_routine_kinds(plan);
    const auto routine_locations = collect_library_export_routine_locations(plan);
    stream << "manifest_version=1\n";
    stream << "manifest_value_encoding=backslash-v1\n";
    stream << "output_kind=fll\n";
    stream << "library_file=" << quote_manifest_value(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path).filename())) << "\n";
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
    stream.imbue(std::locale::classic());
    const auto parameter_counts = collect_library_export_parameter_counts(plan);
    const auto parameter_names = collect_library_export_parameter_names(plan);
    const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
    const auto routine_kinds = collect_library_export_routine_kinds(plan);
    const auto routine_locations = collect_library_export_routine_locations(plan);
    stream << "manifest_version=1\n";
    stream << "manifest_value_encoding=backslash-v1\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "library_file=" << quote_manifest_value(copperfin::platform::path_to_utf8_string(
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path).filename())) << "\n";
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

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime
