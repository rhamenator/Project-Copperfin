// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/code_page.h"

#include "copperfin/platform/environment.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cerrno>
#include <limits>
#include <string_view>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <iconv.h>
#include <langinfo.h>
#endif

namespace copperfin::platform {

namespace {

std::string trim_locale_text(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }
    return value;
}

std::string uppercase_locale_text(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

std::optional<int> parse_decimal_code_page(const std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    int code_page = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), code_page);
    if (error != std::errc{} || end != text.data() + text.size() || code_page <= 0) {
        return std::nullopt;
    }
    return code_page;
}

std::optional<int> parse_codeset_token(const std::string_view codeset) {
    if (codeset == "UTF-8" || codeset == "UTF8") {
        return 65001;
    }
    if (codeset == "US-ASCII" || codeset == "ASCII" ||
        codeset == "ANSI_X3.4-1968" || codeset == "C" || codeset == "POSIX") {
        return 20127;
    }

    if (const auto numeric = parse_decimal_code_page(codeset); numeric.has_value()) {
        return numeric;
    }

    constexpr std::string_view prefixes[] = {
        "CP-", "CP", "WINDOWS-", "WINDOWS_", "IBM-", "IBM",
    };
    for (const std::string_view prefix : prefixes) {
        if (codeset.starts_with(prefix)) {
            return parse_decimal_code_page(codeset.substr(prefix.size()));
        }
    }
    return std::nullopt;
}

#if !defined(_WIN32)
std::optional<std::string> iconv_encoding_name_for_code_page(const int code_page) {
    switch (code_page) {
        case 437: return "CP437";
        case 620: return "CP620";
        case 737: return "CP737";
        case 850: return "CP850";
        case 852: return "CP852";
        case 857: return "CP857";
        case 861: return "CP861";
        case 865: return "CP865";
        case 866: return "CP866";
        case 874: return "CP874";
        case 895: return "CP895";
        case 932: return "CP932";
        case 936: return "CP936";
        case 949: return "CP949";
        case 950: return "CP950";
        case 1250: return "CP1250";
        case 1251: return "CP1251";
        case 1252: return "CP1252";
        case 1253: return "CP1253";
        case 1254: return "CP1254";
        case 1255: return "CP1255";
        case 1256: return "CP1256";
        case 10000: return "MACINTOSH";
        case 10006: return "MACGREEK";
        case 10007: return "MACCYRILLIC";
        case 10029: return "MACCENTRALEUROPE";
        default: return std::nullopt;
    }
}
#endif

}  // namespace

std::optional<int> parse_posix_locale_code_page(std::string locale_or_codeset) {
    locale_or_codeset = uppercase_locale_text(trim_locale_text(std::move(locale_or_codeset)));
    if (locale_or_codeset.empty()) {
        return std::nullopt;
    }

    if (const std::size_t modifier = locale_or_codeset.find('@'); modifier != std::string::npos) {
        locale_or_codeset.erase(modifier);
    }
    locale_or_codeset = trim_locale_text(std::move(locale_or_codeset));
    if (locale_or_codeset.empty()) {
        return std::nullopt;
    }

    if (const auto direct = parse_codeset_token(locale_or_codeset); direct.has_value()) {
        return direct;
    }

    const std::size_t separator = locale_or_codeset.find('.');
    if (separator == std::string::npos || separator + 1U >= locale_or_codeset.size()) {
        return std::nullopt;
    }
    return parse_codeset_token(locale_or_codeset.substr(separator + 1U));
}

int resolve_posix_host_code_page(
    const std::optional<std::string>& nl_codeset,
    const std::array<std::optional<std::string>, 3U>& locale_candidates) {
    if (nl_codeset.has_value()) {
        if (const auto parsed = parse_posix_locale_code_page(*nl_codeset); parsed.has_value()) {
            return *parsed;
        }
    }
    for (const auto& candidate : locale_candidates) {
        if (candidate.has_value()) {
            if (const auto parsed = parse_posix_locale_code_page(*candidate); parsed.has_value()) {
                return *parsed;
            }
        }
    }
    return 1252;
}

int host_code_page() {
#if defined(_WIN32)
    const UINT active_code_page = ::GetACP();
    return active_code_page == 0U ? 1252 : static_cast<int>(active_code_page);
#else
    std::optional<std::string> system_codeset;
    if (const char* codeset = nl_langinfo(CODESET); codeset != nullptr) {
        system_codeset = codeset;
    }
    return resolve_posix_host_code_page(system_codeset, {
        read_environment_variable("LC_ALL"),
        read_environment_variable("LC_CTYPE"),
        read_environment_variable("LANG"),
    });
#endif
}

int host_oem_code_page() {
#if defined(_WIN32)
    const UINT oem_code_page = ::GetOEMCP();
    return oem_code_page == 0U ? host_code_page() : static_cast<int>(oem_code_page);
#else
    return host_code_page();
#endif
}

std::optional<std::string> convert_code_page_bytes(
    const int source_code_page,
    const int target_code_page,
    const std::string& input) {
#if defined(_WIN32)
    if (input.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return std::nullopt;
    }
    const int input_size = static_cast<int>(input.size());
    const int wide_count = ::MultiByteToWideChar(
        static_cast<UINT>(source_code_page), 0, input.data(), input_size, nullptr, 0);
    if (wide_count <= 0) {
        return std::nullopt;
    }
    std::wstring wide_text(static_cast<std::size_t>(wide_count), L'\0');
    if (::MultiByteToWideChar(static_cast<UINT>(source_code_page), 0, input.data(), input_size,
            wide_text.data(), wide_count) <= 0) {
        return std::nullopt;
    }
    const int byte_count = ::WideCharToMultiByte(
        static_cast<UINT>(target_code_page), 0, wide_text.data(), wide_count,
        nullptr, 0, nullptr, nullptr);
    if (byte_count <= 0) {
        return std::nullopt;
    }
    std::string output(static_cast<std::size_t>(byte_count), '\0');
    if (::WideCharToMultiByte(static_cast<UINT>(target_code_page), 0, wide_text.data(), wide_count,
            output.data(), byte_count, nullptr, nullptr) <= 0) {
        return std::nullopt;
    }
    return output;
#else
    const auto source_name = iconv_encoding_name_for_code_page(source_code_page);
    const auto target_name = iconv_encoding_name_for_code_page(target_code_page);
    if (!source_name.has_value() || !target_name.has_value()) {
        return std::nullopt;
    }
    iconv_t converter = iconv_open(target_name->c_str(), source_name->c_str());
    if (converter == reinterpret_cast<iconv_t>(-1)) {
        return std::nullopt;
    }
    std::string output(std::max<std::size_t>(input.size() * 4U, 16U), '\0');
    char* input_buffer = const_cast<char*>(input.data());
    std::size_t input_remaining = input.size();
    char* output_buffer = output.data();
    std::size_t output_remaining = output.size();
    while (true) {
        const std::size_t result = iconv(
            converter, &input_buffer, &input_remaining, &output_buffer, &output_remaining);
        if (result != static_cast<std::size_t>(-1)) {
            break;
        }
        if (errno != E2BIG) {
            iconv_close(converter);
            return std::nullopt;
        }
        const std::size_t bytes_written = output.size() - output_remaining;
        output.resize(output.size() * 2U, '\0');
        output_buffer = output.data() + bytes_written;
        output_remaining = output.size() - bytes_written;
    }
    iconv_close(converter);
    output.resize(output.size() - output_remaining);
    return output;
#endif
}

}  // namespace copperfin::platform
