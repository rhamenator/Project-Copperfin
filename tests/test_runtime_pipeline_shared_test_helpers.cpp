// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_support.h"
#include "test_environment_support.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#else
#include <dlfcn.h>
extern char** environ;
#endif

namespace cf_test_runtime_pipeline {
int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_materialization(
    const copperfin::runtime::RuntimeMaterializeResult& result,
    const std::string& message) {
    expect(
        result.ok,
        result.ok ? message : message + "; error: " + result.error);
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

bool paths_refer_to_same_filesystem_entry(
    const std::filesystem::path& actual,
    const std::filesystem::path& expected) {
    if (actual.empty() || expected.empty()) {
        return actual.empty() && expected.empty();
    }

    std::error_code equivalent_error;
    if (std::filesystem::equivalent(actual, expected, equivalent_error) &&
        !equivalent_error) {
        return true;
    }
    return actual.lexically_normal() == expected.lexically_normal();
}

std::string decode_manifest_value(const std::string& value) {
    std::string decoded;
    decoded.reserve(value.size());
    for (std::size_t index = 0U; index < value.size(); ++index) {
        if (value[index] != '\\' || index + 1U >= value.size()) {
            decoded.push_back(value[index]);
            continue;
        }
        const char escaped = value[++index];
        if (escaped == '\\') {
            decoded.push_back('\\');
        } else if (escaped == 'n') {
            decoded.push_back('\n');
        } else if (escaped == 'r') {
            decoded.push_back('\r');
        } else if (escaped == '|') {
            decoded.push_back('|');
        } else {
            decoded.push_back('\\');
            decoded.push_back(escaped);
        }
    }
    return decoded;
}

#if defined(_WIN32)
bool create_windows_junction(
    const std::filesystem::path& link,
    const std::filesystem::path& target) {
    struct JunctionHeader {
        DWORD tag;
        WORD data_length;
        WORD reserved;
        WORD substitute_offset;
        WORD substitute_length;
        WORD print_offset;
        WORD print_length;
    };

    const std::wstring target_path = std::filesystem::absolute(target).native();
    const std::wstring substitute = L"\\??\\" + target_path;
    const std::wstring print = target_path;
    const std::size_t path_characters =
        substitute.size() + 1U + print.size() + 1U;
    const std::size_t path_bytes = path_characters * sizeof(wchar_t);
    if (path_bytes + sizeof(JunctionHeader) >
        static_cast<std::size_t>(std::numeric_limits<WORD>::max())) {
        return false;
    }

    std::error_code error;
    std::filesystem::create_directory(link, error);
    if (error) {
        return false;
    }
    const HANDLE handle = ::CreateFileW(
        link.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        std::filesystem::remove(link, error);
        return false;
    }

    std::vector<std::byte> buffer(sizeof(JunctionHeader) + path_bytes);
    auto* header = reinterpret_cast<JunctionHeader*>(buffer.data());
    header->tag = IO_REPARSE_TAG_MOUNT_POINT;
    header->data_length = static_cast<WORD>(8U + path_bytes);
    header->reserved = 0U;
    header->substitute_offset = 0U;
    header->substitute_length =
        static_cast<WORD>(substitute.size() * sizeof(wchar_t));
    header->print_offset =
        static_cast<WORD>((substitute.size() + 1U) * sizeof(wchar_t));
    header->print_length = static_cast<WORD>(print.size() * sizeof(wchar_t));
    auto* path_buffer = reinterpret_cast<wchar_t*>(
        buffer.data() + sizeof(JunctionHeader));
    std::memcpy(
        path_buffer,
        substitute.c_str(),
        (substitute.size() + 1U) * sizeof(wchar_t));
    std::memcpy(
        path_buffer + substitute.size() + 1U,
        print.c_str(),
        (print.size() + 1U) * sizeof(wchar_t));

    DWORD returned = 0U;
    const bool created = ::DeviceIoControl(
        handle,
        FSCTL_SET_REPARSE_POINT,
        buffer.data(),
        static_cast<DWORD>(buffer.size()),
        nullptr,
        0U,
        &returned,
        nullptr) != 0;
    (void)::CloseHandle(handle);
    if (!created) {
        std::filesystem::remove(link, error);
    }
    return created;
}

bool create_windows_drive_mapping(
    const std::filesystem::path& target,
    std::filesystem::path& drive_root) {
    drive_root.clear();
    const std::wstring target_path = std::filesystem::absolute(target).native();
    const std::wstring raw_target = L"\\??\\" + target_path;
    for (wchar_t letter = L'Z'; letter >= L'D'; --letter) {
        const std::wstring device{letter, L':'};
        std::array<wchar_t, 2U> existing{};
        if (::QueryDosDeviceW(
                device.c_str(),
                existing.data(),
                static_cast<DWORD>(existing.size())) != 0U ||
            ::GetLastError() != ERROR_FILE_NOT_FOUND) {
            continue;
        }
        if (::DefineDosDeviceW(
                DDD_RAW_TARGET_PATH | DDD_NO_BROADCAST_SYSTEM,
                device.c_str(),
                raw_target.c_str()) != 0) {
            drive_root = std::filesystem::path(device + L"\\");
            return true;
        }
    }
    return false;
}

bool remove_windows_drive_mapping(
    const std::filesystem::path& target,
    const std::filesystem::path& drive_root) {
    const std::wstring device = drive_root.root_name().native();
    const std::wstring target_path = std::filesystem::absolute(target).native();
    const std::wstring raw_target = L"\\??\\" + target_path;
    return !device.empty() &&
        ::DefineDosDeviceW(
            DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE |
                DDD_RAW_TARGET_PATH | DDD_NO_BROADCAST_SYSTEM,
            device.c_str(),
            raw_target.c_str()) != 0;
}
#endif

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
        } else if (ch == '|') {
            escaped += "\\|";
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
        "-DCOPPERFIN_RUNTIME_BRIDGE_TEST_HOOKS=1",
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

void test_generated_posix_bridge_environment_launch(const std::filesystem::path& wrapper_path) {
#if defined(_WIN32)
    namespace fs = std::filesystem;
    using TestLaunch = int (*)(const char*, const char*, const char*, const char*, const char*, const char*);
    HMODULE module = ::LoadLibraryW(wrapper_path.c_str());
    expect(module != nullptr, "generated Windows bridge test seam should load the compiled wrapper");
    if (module == nullptr) {
        return;
    }
    const auto launch = reinterpret_cast<TestLaunch>(
        ::GetProcAddress(module, "copperfin_runtime_bridge_test_launch_environment"));
    expect(launch != nullptr, "generated Windows bridge test seam should export the environment launcher");
    if (launch == nullptr) {
        (void)::FreeLibrary(module);
        return;
    }

    const fs::path temp_root = fs::temp_directory_path();
    const fs::path sentinel_path =
        temp_root / ("copperfin-inherited-handle-" + std::to_string(::GetCurrentProcessId()) + ".txt");
    const fs::path output_path =
        temp_root / ("copperfin-inherited-handle-" + std::to_string(::GetCurrentProcessId()) + ".out");
    SECURITY_ATTRIBUTES security_attributes{};
    security_attributes.nLength = sizeof(security_attributes);
    security_attributes.bInheritHandle = TRUE;
    const HANDLE sentinel = ::CreateFileW(
        sentinel_path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security_attributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    expect(sentinel != INVALID_HANDLE_VALUE,
           "#4344: Windows handle-inheritance regression should create an inheritable sentinel");
    if (sentinel == INVALID_HANDLE_VALUE) {
        (void)::FreeLibrary(module);
        return;
    }
    BY_HANDLE_FILE_INFORMATION sentinel_information{};
    const bool sentinel_inspected =
        ::GetFileInformationByHandle(sentinel, &sentinel_information) != 0;
    expect(sentinel_inspected,
           "#4344: Windows handle-inheritance regression should inspect the sentinel identity");
    if (!sentinel_inspected) {
        (void)::CloseHandle(sentinel);
        (void)::DeleteFileW(sentinel_path.c_str());
        (void)::FreeLibrary(module);
        return;
    }

    wchar_t executable_buffer[32768]{};
    const DWORD executable_length = ::GetModuleFileNameW(
        nullptr,
        executable_buffer,
        static_cast<DWORD>(std::size(executable_buffer)));
    expect(executable_length > 0U && executable_length < std::size(executable_buffer),
           "#4344: Windows handle-inheritance regression should resolve its test executable");
    if (executable_length == 0U || executable_length >= std::size(executable_buffer)) {
        (void)::CloseHandle(sentinel);
        (void)::DeleteFileW(sentinel_path.c_str());
        (void)::FreeLibrary(module);
        return;
    }

    const std::string probe_value =
        std::to_string(reinterpret_cast<std::uintptr_t>(sentinel)) + "," +
        std::to_string(sentinel_information.dwVolumeSerialNumber) + "," +
        std::to_string(sentinel_information.nFileIndexHigh) + "," +
        std::to_string(sentinel_information.nFileIndexLow);
    const std::string executable_path = copperfin::platform::path_to_utf8_string(
        fs::path(std::wstring(executable_buffer, executable_length)));
    const std::string output_path_string = output_path.string();
    const std::string working_directory = temp_root.string();
    const int exit_code = launch(
        executable_path.c_str(),
        output_path_string.c_str(),
        working_directory.c_str(),
        "COPPERFIN_INHERITED_HANDLE_PROBE",
        probe_value.c_str(),
        "--copperfin-inherited-handle-probe");
    expect(exit_code == 0,
           "#4344: Windows generated bridge should launch the handle probe without inheriting the sentinel");
    const std::string probe_output = read_text(output_path);
    expect(probe_output == "not-inherited\n" || probe_output == "not-inherited\r\n",
           "#4344: Windows generated bridge child should not observe an unrelated inheritable handle");

    (void)::CloseHandle(sentinel);
    expect(::DeleteFileW(sentinel_path.c_str()) != 0,
           "#4344: Windows handle-inheritance regression should release the sentinel after the child exits");
    std::error_code ignored;
    fs::remove(output_path, ignored);
    (void)::FreeLibrary(module);
#else
    namespace fs = std::filesystem;
    using TestLaunch = int (*)(const char*, const char*, const char*, const char*, const char*, const char*);
    void* module = dlopen(wrapper_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    expect(module != nullptr, "generated POSIX bridge test seam should load the compiled wrapper");
    if (module == nullptr) {
        return;
    }
    const auto launch = reinterpret_cast<TestLaunch>(dlsym(module, "copperfin_runtime_bridge_test_launch_environment"));
    expect(launch != nullptr, "generated POSIX bridge test seam should export the environment launcher");
    if (launch == nullptr) {
        dlclose(module);
        return;
    }

    const fs::path output_path = wrapper_path.parent_path() / "environment-test.bin";
    const std::string inherited_value = "inherited-\xC3\xA9";
    std::vector<std::string> inherited_entries = {
        "COPPERFIN_TEST_INHERITED=" + inherited_value,
        "COPPERFIN_TEST_DUPLICATE=first",
        "COPPERFIN_TEST_DUPLICATE=second"};
    std::vector<char*> inherited_pointers;
    inherited_pointers.reserve(inherited_entries.size() + 1U);
    for (auto& entry : inherited_entries) {
        inherited_pointers.push_back(entry.data());
    }
    inherited_pointers.push_back(nullptr);
    char** original_environment = environ;
    environ = inherited_pointers.data();
    const int exit_code = launch(
        "/usr/bin/env",
        output_path.c_str(),
        "/tmp",
        "COPPERFIN_TEST_DUPLICATE",
        "override",
        "-0");
    environ = original_environment;

    expect(exit_code == 0, "generated POSIX bridge test seam should report the execve child exit code");
    const std::string output = read_text(output_path);
    const std::string expected_inherited = "COPPERFIN_TEST_INHERITED=" + inherited_value;
    expect(output.find(expected_inherited + '\0') != std::string::npos,
           "generated POSIX bridge should preserve UTF-8 inherited environment values");
    expect(output.find("COPPERFIN_TEST_DUPLICATE=override\0") != std::string::npos,
           "generated POSIX bridge should apply the last override value");
    expect(output.find("COPPERFIN_TEST_DUPLICATE=first\0") == std::string::npos &&
               output.find("COPPERFIN_TEST_DUPLICATE=second\0") == std::string::npos,
           "generated POSIX bridge should collapse duplicate inherited keys deterministically");
    const fs::path working_directory = fs::temp_directory_path();
    const fs::path working_directory_output = wrapper_path.parent_path() / "working-directory-test.txt";
    const int working_directory_exit_code = launch(
        "/bin/pwd",
        working_directory_output.c_str(),
        working_directory.c_str(),
        "",
        "",
        "");
    expect(working_directory_exit_code == 0,
           "generated POSIX bridge test seam should preserve the child working directory launch contract");
    const std::string expected_working_directory = fs::weakly_canonical(working_directory).string() + "\n";
    expect(read_text(working_directory_output) == expected_working_directory,
           "generated POSIX bridge should launch the child in the requested working directory");
    std::error_code ignored;
    fs::remove(output_path, ignored);
    fs::remove(working_directory_output, ignored);
    dlclose(module);
#endif
}

void test_generated_bridge_runtime_host_verification(const std::filesystem::path& wrapper_path) {
    namespace fs = std::filesystem;
    using TestVerify = int (*)(const char*, const char*, const char*, const char*, const char*, const char*);
#if defined(_WIN32)
    HMODULE module = ::LoadLibraryW(wrapper_path.c_str());
    expect(module != nullptr, "generated Windows bridge verifier seam should load the compiled wrapper");
    if (module == nullptr) {
        return;
    }
    const auto verify = reinterpret_cast<TestVerify>(
        ::GetProcAddress(module, "copperfin_runtime_bridge_test_launch_environment"));
#else
    void* module = dlopen(wrapper_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    expect(module != nullptr, "generated POSIX bridge verifier seam should load the compiled wrapper");
    if (module == nullptr) {
        return;
    }
    const auto verify = reinterpret_cast<TestVerify>(
        dlsym(module, "copperfin_runtime_bridge_test_launch_environment"));
#endif
    expect(verify != nullptr, "generated bridge verifier seam should expose the existing test hook");
    if (verify == nullptr) {
#if defined(_WIN32)
        (void)::FreeLibrary(module);
#else
        dlclose(module);
#endif
        return;
    }

    const fs::path temp_root = wrapper_path.parent_path() / "runtime-host-verification-test";
    const fs::path host_path = temp_root / "copperfin_runtime_host";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path outside_path = temp_root / "outside-host";
    const fs::path redirected_path = temp_root / "redirected-host";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    write_text(host_path, "verified-runtime-host\n");
    const auto digest = copperfin::security::sha256_hex_for_file(host_path.string());
    expect(digest.ok, "generated bridge verification fixture should hash the runtime host");
    write_text(manifest_path, "runtime_host_sha256=" + digest.hex_digest + "\n");
    const std::string temp_root_string = temp_root.string();
    const std::string host_path_string = host_path.string();
    const std::string manifest_path_string = manifest_path.string();
    const std::string verify_argument = "--copperfin-verify-runtime-host";
    expect(verify(
               host_path_string.c_str(), manifest_path_string.c_str(), temp_root_string.c_str(),
               "", "", verify_argument.c_str()) == 0,
           "generated bridge should accept an unchanged runtime host with a matching digest");

    write_text(host_path, "tampered-runtime-host\n");
    expect(verify(
               host_path_string.c_str(), manifest_path_string.c_str(), temp_root_string.c_str(),
               "", "", verify_argument.c_str()) != 0,
           "generated bridge should reject a runtime host whose digest no longer matches");

    write_text(outside_path, "outside-runtime-host\n");
    const auto outside_digest = copperfin::security::sha256_hex_for_file(outside_path.string());
    write_text(manifest_path, "runtime_host_sha256=" + outside_digest.hex_digest + "\n");
    fs::create_symlink(outside_path, redirected_path, ignored);
    if (!ignored) {
        const std::string redirected_path_string = redirected_path.string();
        expect(verify(
                   redirected_path_string.c_str(), manifest_path_string.c_str(), temp_root_string.c_str(),
                   "", "", verify_argument.c_str()) != 0,
               "generated bridge should reject a redirected runtime-host path");
    }

    fs::remove_all(temp_root, ignored);
#if defined(_WIN32)
    (void)::FreeLibrary(module);
#else
    dlclose(module);
#endif
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
        if (symbol == "copperfin_runtime_bridge_test_launch_environment") {
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

    expect_materialization(result, "library warning-path package should materialize for " + extension + " outputs");
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
