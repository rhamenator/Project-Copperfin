// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/printer.h"

#include "copperfin/platform/bounded_process.h"
#include "copperfin/platform/executable_path.h"
#include "copperfin/platform/path.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winspool.h>
#endif

namespace copperfin::platform {
namespace {

std::string trim_copy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char character) {
        return std::isspace(character) == 0;
    }));
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }
    return value;
}

std::string ascii_lower_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

void append_unique(std::vector<std::string>& names, std::set<std::string>& seen, std::string name) {
    name = trim_copy(std::move(name));
    if (!name.empty() && seen.insert(ascii_lower_copy(name)).second) {
        names.push_back(std::move(name));
    }
}

#if defined(_WIN32)
std::string utf8_from_wide(const wchar_t* value) {
    if (value == nullptr || *value == L'\0') {
        return {};
    }
    const int required = ::WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, value, -1, nullptr, 0, nullptr, nullptr);
    if (required <= 1) {
        return {};
    }
    std::string result(static_cast<std::size_t>(required), '\0');
    if (::WideCharToMultiByte(
            CP_UTF8, WC_ERR_INVALID_CHARS, value, -1,
            result.data(), required, nullptr, nullptr) != required) {
        return {};
    }
    result.resize(static_cast<std::size_t>(required - 1));
    return result;
}

std::vector<std::string> enumerate_windows_printers() {
    DWORD required = 0U;
    DWORD count = 0U;
    constexpr DWORD flags = PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS;
    (void)::EnumPrintersW(flags, nullptr, 4U, nullptr, 0U, &required, &count);
    if (required == 0U) {
        return {};
    }
    std::vector<unsigned char> storage(required);
    if (::EnumPrintersW(
            flags, nullptr, 4U, storage.data(), required, &required, &count) == FALSE) {
        return {};
    }
    const auto* printers = reinterpret_cast<const PRINTER_INFO_4W*>(storage.data());
    std::vector<std::string> names;
    std::set<std::string> seen;
    for (DWORD index = 0U; index < count; ++index) {
        append_unique(names, seen, utf8_from_wide(printers[index].pPrinterName));
    }
    return names;
}
#else
std::string printer_name_from_lpstat_line(const std::string& line) {
    const std::string trimmed = trim_copy(line);
    if (trimmed.empty()) {
        return {};
    }
    const std::size_t begin = trimmed.rfind("printer ", 0U) == 0U ? 8U : 0U;
    const std::size_t end = trimmed.find_first_of(" \t", begin);
    return end == std::string::npos
        ? trimmed.substr(begin)
        : trimmed.substr(begin, end - begin);
}

std::vector<std::string> parse_lpstat_output(const std::string& output) {
    std::vector<std::string> names;
    std::set<std::string> seen;
    std::size_t begin = 0U;
    while (begin <= output.size()) {
        const std::size_t end = output.find_first_of("\r\n", begin);
        append_unique(names, seen, printer_name_from_lpstat_line(
            output.substr(begin, end == std::string::npos ? std::string::npos : end - begin)));
        if (end == std::string::npos) {
            break;
        }
        begin = end + 1U;
        if (output[end] == '\r' && begin < output.size() && output[begin] == '\n') {
            ++begin;
        }
    }
    return names;
}

std::vector<std::string> run_lpstat(const std::string& argument) {
    namespace fs = std::filesystem;
    const fs::path executable = resolve_executable_invocation_path("lpstat");
    std::error_code status_error;
    if (!executable.is_absolute() || !fs::is_regular_file(executable, status_error) || status_error) {
        return {};
    }
    BoundedProcessRequest request;
    request.executable_path = path_to_utf8_string(executable);
    request.arguments = {argument};
    request.working_directory = path_to_utf8_string(fs::current_path());
    request.timeout_ms = 2000U;
    request.stdout_limit_bytes = 256U * 1024U;
    request.stderr_limit_bytes = 64U * 1024U;
    const BoundedProcessResult result = run_bounded_process(request);
    return result.completed() && result.exit_code == 0
        ? parse_lpstat_output(result.standard_output)
        : std::vector<std::string>{};
}

std::vector<std::string> enumerate_posix_printers() {
    std::vector<std::string> names = run_lpstat("-a");
    return names.empty() ? run_lpstat("-p") : names;
}
#endif

}  // namespace

std::vector<std::string> enumerate_printer_names() {
#if defined(_WIN32)
    return enumerate_windows_printers();
#else
    return enumerate_posix_printers();
#endif
}

}  // namespace copperfin::platform
