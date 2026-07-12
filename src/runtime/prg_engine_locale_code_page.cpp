// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "prg_engine_locale_code_page.h"

#include "prg_engine_helpers.h"

#include <charconv>
#include <string_view>

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

}  // namespace copperfin::runtime::detail
