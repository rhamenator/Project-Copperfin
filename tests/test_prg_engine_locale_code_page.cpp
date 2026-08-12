// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/code_page.h"
#include "prg_engine_locale_code_page.h"
#include "prg_engine_test_support.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

namespace {

using copperfin::platform::parse_posix_locale_code_page;
using copperfin::platform::resolve_posix_host_code_page;
using copperfin::platform::convert_code_page_bytes;
using copperfin::runtime::detail::is_lead_byte_for_code_page;
using copperfin::test_support::expect;

void expect_code_page(const std::string& text, std::optional<int> expected) {
    const auto actual = parse_posix_locale_code_page(text);
    expect(actual == expected, "#3961: unexpected code page for locale/codeset '" + text + "'");
}

void test_codeset_and_full_locale_parsing() {
    expect_code_page("UTF-8", 65001);
    expect_code_page("utf8", 65001);
    expect_code_page("C.UTF-8", 65001);
    expect_code_page("en_US.UTF-8", 65001);
    expect_code_page("pt_BR.UTF8@latin", 65001);
    expect_code_page("1252", 1252);
    expect_code_page("CP1251", 1251);
    expect_code_page("cp-1250", 1250);
    expect_code_page("WINDOWS-1252", 1252);
    expect_code_page("windows_1254", 1254);
    expect_code_page("IBM850", 850);
    expect_code_page("ibm-437", 437);
    expect_code_page("en_US.1252", 1252);
    expect_code_page("sr_RS.CP1251@latin", 1251);
    expect_code_page("de_DE.WINDOWS-1252@euro", 1252);
    expect_code_page("C", 20127);
    expect_code_page("POSIX", 20127);
    expect_code_page("ANSI_X3.4-1968", 20127);
}

void test_malformed_values_do_not_scrape_unrelated_digits() {
    expect_code_page("", std::nullopt);
    expect_code_page("en_US", std::nullopt);
    expect_code_page("es_419", std::nullopt);
    expect_code_page("en_US@modifier", std::nullopt);
    expect_code_page("en_US.", std::nullopt);
    expect_code_page("en_US.UTF-8-extra", std::nullopt);
    expect_code_page("locale123", std::nullopt);
    expect_code_page("CP", std::nullopt);
    expect_code_page("CP12x", std::nullopt);
    expect_code_page("0", std::nullopt);
    expect_code_page("999999999999999999999", std::nullopt);
}

void test_posix_fallback_order() {
    using CandidateArray = std::array<std::optional<std::string>, 3U>;

    expect(
        resolve_posix_host_code_page("UTF-8", CandidateArray{"CP1250", "CP1251", "CP1252"}) == 65001,
        "#3961: nl_langinfo codeset should take precedence over environment locales");
    expect(
        resolve_posix_host_code_page("unparseable", CandidateArray{"en_US.CP1250", "CP1251", "CP1252"}) == 1250,
        "#3961: LC_ALL should be the first environment fallback");
    expect(
        resolve_posix_host_code_page(std::nullopt, CandidateArray{"es_419", "sr_RS.CP1251", "CP1252"}) == 1251,
        "#3961: malformed LC_ALL should fall through to LC_CTYPE without scraping territory digits");
    expect(
        resolve_posix_host_code_page(std::nullopt, CandidateArray{std::nullopt, "", "C.UTF-8"}) == 65001,
        "#3961: LANG should be used after missing or empty higher-priority candidates");
    expect(
        resolve_posix_host_code_page("bad", CandidateArray{"es_419", "CP12x", std::nullopt}) == 1252,
        "#3961: unparseable sources should retain the deterministic 1252 fallback");
}

void test_dbcs_lead_byte_ranges() {
    expect(!is_lead_byte_for_code_page(1252, 0x81U), "single-byte code pages must not report lead bytes");
    expect(!is_lead_byte_for_code_page(65001, 0xC3U), "UTF-8 must not use DBCS lead-byte rules");

    expect(!is_lead_byte_for_code_page(932, 0x80U), "CP932 byte below the first lead range must be rejected");
    expect(is_lead_byte_for_code_page(932, 0x81U), "CP932 first lead boundary should be accepted");
    expect(is_lead_byte_for_code_page(932, 0x9FU), "CP932 first lead range upper boundary should be accepted");
    expect(!is_lead_byte_for_code_page(932, 0xA0U), "CP932 gap between lead ranges must be rejected");
    expect(is_lead_byte_for_code_page(932, 0xE0U), "CP932 second lead boundary should be accepted");
    expect(is_lead_byte_for_code_page(932, 0xFCU), "CP932 second lead range upper boundary should be accepted");
    expect(!is_lead_byte_for_code_page(932, 0xFDU), "CP932 byte above the lead range must be rejected");

    for (const int code_page : {936, 949, 950}) {
        expect(!is_lead_byte_for_code_page(code_page, 0x80U), "DBCS byte below the shared lead range must be rejected");
        expect(is_lead_byte_for_code_page(code_page, 0x81U), "DBCS first lead boundary should be accepted");
        expect(is_lead_byte_for_code_page(code_page, 0xFEU), "DBCS last lead boundary should be accepted");
        expect(!is_lead_byte_for_code_page(code_page, 0xFFU), "DBCS byte above the shared lead range must be rejected");
    }
}

void test_platform_code_page_conversion() {
    const auto identity = convert_code_page_bytes(1252, 1252, "Copperfin");
    expect(identity.has_value() && *identity == "Copperfin",
           "platform code-page conversion should preserve representable identity text");

    const auto converted = convert_code_page_bytes(
        437, 1252, std::string(1, static_cast<char>(0x8E)));
    expect(converted.has_value() &&
               *converted == std::string(1, static_cast<char>(0xC4)),
           "platform code-page conversion should preserve the existing CP437-to-1252 mapping");

    expect(!convert_code_page_bytes(99999, 1252, "x").has_value(),
           "platform code-page conversion should reject an unsupported source code page");
}

}  // namespace

int main() {
    test_codeset_and_full_locale_parsing();
    test_malformed_values_do_not_scrape_unrelated_digits();
    test_posix_fallback_order();
    test_dbcs_lead_byte_ranges();
    test_platform_code_page_conversion();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
