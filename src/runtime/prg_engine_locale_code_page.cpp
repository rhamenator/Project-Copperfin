// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_locale_code_page.h"

#include "copperfin/platform/code_page.h"

#include <utility>

namespace copperfin::runtime::detail {

std::optional<int> parse_posix_locale_code_page(std::string locale_or_codeset) {
    return copperfin::platform::parse_posix_locale_code_page(std::move(locale_or_codeset));
}

int resolve_posix_host_code_page(
    const std::optional<std::string>& nl_codeset,
    const std::array<std::optional<std::string>, 3U>& locale_candidates) {
    return copperfin::platform::resolve_posix_host_code_page(nl_codeset, locale_candidates);
}

int default_host_code_page() {
    return copperfin::platform::host_code_page();
}

int default_host_oem_code_page() {
    return copperfin::platform::host_oem_code_page();
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
