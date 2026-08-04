// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_locale_code_page.h"

#include "copperfin/platform/environment.h"
#include "prg_engine_helpers.h"

#include <charconv>
#include <string_view>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <langinfo.h>
#endif

namespace copperfin::runtime::detail {

namespace {

std::optional<int> parse_decimal_code_page(std::string_view text) {
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

std::optional<int> parse_codeset_token(std::string_view codeset) {
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

    const std::string_view prefixes[] = {
        "CP-",
        "CP",
        "WINDOWS-",
        "WINDOWS_",
        "IBM-",
        "IBM",
    };
    for (const std::string_view prefix : prefixes) {
        if (codeset.starts_with(prefix)) {
            return parse_decimal_code_page(codeset.substr(prefix.size()));
        }
    }
    return std::nullopt;
}

}  // namespace

std::optional<int> parse_posix_locale_code_page(std::string locale_or_codeset) {
    locale_or_codeset = uppercase_copy(trim_copy(std::move(locale_or_codeset)));
    if (locale_or_codeset.empty()) {
        return std::nullopt;
    }

    if (const std::size_t modifier = locale_or_codeset.find('@'); modifier != std::string::npos) {
        locale_or_codeset.erase(modifier);
    }
    locale_or_codeset = trim_copy(std::move(locale_or_codeset));
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
        if (!candidate.has_value()) {
            continue;
        }
        if (const auto parsed = parse_posix_locale_code_page(*candidate); parsed.has_value()) {
            return *parsed;
        }
    }
    return 1252;
}

int default_host_code_page() {
#if defined(_WIN32)
    const UINT active_code_page = GetACP();
    return active_code_page == 0U ? 1252 : static_cast<int>(active_code_page);
#else
    std::optional<std::string> system_codeset;
    if (const char* codeset = nl_langinfo(CODESET); codeset != nullptr) {
        system_codeset = codeset;
    }

    const std::array<std::optional<std::string>, 3U> locale_candidates = {
        copperfin::platform::read_environment_variable("LC_ALL"),
        copperfin::platform::read_environment_variable("LC_CTYPE"),
        copperfin::platform::read_environment_variable("LANG"),
    };
    return resolve_posix_host_code_page(system_codeset, locale_candidates);
#endif
}

int default_host_oem_code_page() {
#if defined(_WIN32)
    const UINT oem_code_page = GetOEMCP();
    return oem_code_page == 0U ? default_host_code_page() : static_cast<int>(oem_code_page);
#else
    return default_host_code_page();
#endif
}

bool is_supported_vfp_code_page(int code_page) {
    switch (code_page) {
        case 437:
        case 620:
        case 737:
        case 850:
        case 852:
        case 857:
        case 861:
        case 865:
        case 866:
        case 874:
        case 895:
        case 932:
        case 936:
        case 949:
        case 950:
        case 1250:
        case 1251:
        case 1252:
        case 1253:
        case 1254:
        case 1255:
        case 1256:
        case 10000:
        case 10006:
        case 10007:
        case 10029:
            return true;
        default:
            return false;
    }
}

bool is_lead_byte_for_code_page(const int code_page, const unsigned char byte) {
    switch (code_page) {
        case 932:
            return (byte >= 0x81U && byte <= 0x9FU) || (byte >= 0xE0U && byte <= 0xFCU);
        case 936:
        case 949:
        case 950:
            return byte >= 0x81U && byte <= 0xFEU;
        default:
            return false;
    }
}

}  // namespace copperfin::runtime::detail
