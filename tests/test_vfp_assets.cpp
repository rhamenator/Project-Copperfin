// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/platform/path.h"
#include "copperfin/vfp/access_container.h"
#include "copperfin/vfp/access_msysobjects.h"
#include "copperfin/vfp/access_table_definition.h"
#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/cdx_header.h"
#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/index_probe.h"
#include "test_environment_support.h"
#include "test_vfp_assets_support.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <string_view>
#include <vector>

namespace {

using namespace copperfin::tests::vfp_assets_support;

std::vector<std::uint8_t> make_synthetic_cdx_family_bytes(bool include_second_tag, bool include_for_expression) {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(bytes, 1026U, include_second_tag ? 2U : 1U);

    if (include_second_tag) {
        write_le_u32(bytes, 1028U, 11U * 512U);
        write_le_u32(bytes, 1032U, 4U * 512U);
        write_le_u16(bytes, 11U * 512U, 0x0001U);
        write_le_u16(bytes, (11U * 512U) + 2U, 1U);
        write_le_u16(bytes, 4U * 512U, 0x0003U);
        write_le_u16(bytes, (4U * 512U) + 2U, 2U);
        write_ascii(bytes, (3U * 512U) - 20U, "CUSTOMER_I");
        write_ascii(bytes, (3U * 512U) - 10U, "COMPANY_NA");
        write_ascii(bytes, (4U * 512U) + 24U, "UPPER(company_name)");
        write_ascii(bytes, (11U * 512U) + 24U, "customer_id");
    } else {
        write_le_u32(bytes, 1028U, 4U * 512U);
        write_le_u16(bytes, 4U * 512U, 0x0003U);
        write_le_u16(bytes, (4U * 512U) + 2U, 1U);
        write_ascii(bytes, (3U * 512U) - 10U, "NAME");
        write_ascii(bytes, (4U * 512U) + 24U, "UPPER(NAME)");
    }

    if (include_for_expression) {
        write_ascii(bytes, 5U * 512U, "DELETED() = .F.");
    }

    return bytes;
}

std::vector<std::uint8_t> make_synthetic_cdx_bytes_with_decoys() {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(bytes, 1026U, 1U);
    write_le_u32(bytes, 1028U, 11U * 512U);

    write_ascii(bytes, (3U * 512U) - 10U, "CUSTOMER_I");
    write_ascii(bytes, (3U * 512U) + 24U, "customer_invoice_id");
    write_ascii(bytes, (4U * 512U) + 24U, "invoice_deleted = .F.");
    write_ascii(bytes, (11U * 512U) + 24U, "customer_id");
    write_ascii(bytes, (12U * 512U) + 16U, "DELETED() = .F.");

    return bytes;
}

std::vector<std::uint8_t> make_synthetic_cdx_bytes_with_descriptive_tag_name() {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(bytes, 1026U, 1U);
    write_le_u32(bytes, 1028U, 4U * 512U);

    write_ascii(bytes, (3U * 512U) - 10U, "FULLNAME");
    write_ascii(bytes, (4U * 512U) + 24U, "UPPER(LAST+FIRST)");
    return bytes;
}

std::vector<std::uint8_t> make_synthetic_cdx_bytes_with_plain_field_expression() {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(bytes, 1026U, 1U);
    write_le_u32(bytes, 1028U, 4U * 512U);

    write_ascii(bytes, (3U * 512U) - 10U, "NAME");
    write_ascii(bytes, (4U * 512U) + 24U, "NAME");
    return bytes;
}

std::vector<std::uint8_t> make_synthetic_mdx_bytes(bool include_decoy_text) {
    constexpr std::uint16_t block_size = 512U;
    std::vector<std::uint8_t> bytes(6U * block_size, 0U);

    // Header block: real dBase IV MDX format fields at documented byte offsets.
    bytes[0] = 0x02U;                                 // version = dBase IV
    write_le_u16(bytes, 20U, block_size);             // base_block_size = 512
    // block_size_adder (bytes 22-23) stays zero → effective_block_size = 512
    bytes[24] = 0x01U;                                // production index flag
    bytes[25] = 48U;                                  // tag_slots (max 48)
    bytes[26] = 32U;                                  // tag_entry_size = 32
    write_le_u16(bytes, 28U, 2U);                     // tags_in_use = 2
    write_le_u32(bytes, 32U, 6U);                     // pages_in_file = 6

    // Tag table starts at byte 512 (= effective_block_size).
    // Entry 0 (bytes 512–543): tag name at entry+4 = byte 516.
    write_le_u32(bytes, block_size + 0U, 2U);         // tag_header_page_num = 2
    write_ascii(bytes, block_size + 4U, "NAME_TAG");  // 11-byte null-padded name field
    bytes[block_size + 15U] = static_cast<std::uint8_t>('C');
    bytes[block_size + 16U] = 0x01U;
    bytes[block_size + 17U] = 0x00U;
    bytes[block_size + 18U] = 0x00U;
    bytes[block_size + 20U] = static_cast<std::uint8_t>('C');

    // Entry 1 (bytes 544–575): tag name at entry+4 = byte 548.
    write_le_u32(bytes, block_size + 32U, 3U);        // tag_header_page_num = 3
    write_ascii(bytes, block_size + 36U, "CITYSTATE"); // 11-byte null-padded name field
    bytes[block_size + 47U] = static_cast<std::uint8_t>('C');
    bytes[block_size + 48U] = 0x02U;
    bytes[block_size + 49U] = 0x00U;
    bytes[block_size + 50U] = 0x00U;
    bytes[block_size + 52U] = static_cast<std::uint8_t>('C');

    const std::size_t first_tag_header = 2U * block_size;
    write_le_u16(bytes, first_tag_header + 0U, 0x0001U);
    write_le_u16(bytes, first_tag_header + 2U, 9U);
    write_ascii(bytes, first_tag_header + 24U, "UPPER(NAME)");
    write_ascii(bytes, first_tag_header + 80U, "DELETED() = .F.");

    const std::size_t second_tag_header = 3U * block_size;
    write_le_u16(bytes, second_tag_header + 0U, 0x0001U);
    write_le_u16(bytes, second_tag_header + 2U, 7U);
    write_ascii(bytes, second_tag_header + 24U, "UPPER(CITY+STATE)");
    write_ascii(bytes, second_tag_header + 88U, "STATE = 'WA'");

    if (include_decoy_text) {
        // Decoys placed outside the tag-descriptor positions so the real-format parser ignores them.
        write_ascii(bytes, 128U, "HDRTEXT");
        write_ascii(bytes, block_size + 200U, "LATETEXT");
        write_ascii(bytes, 2U * block_size + 100U, "BLOCKTEXT");
    }
    return bytes;
}

void test_parse_dbf_header() {
    const std::locale grouping_locale(std::locale::classic(), new grouped_numpunct());
    global_locale_guard locale_guard(grouping_locale);
    const auto bytes = make_vfp_header();
    const auto result = copperfin::vfp::parse_dbf_header(bytes);

    expect(result.ok, "parse_dbf_header should succeed for a valid synthetic header");
    expect(result.header.version == 0x30U, "version should be parsed");
    expect(result.header.record_count == 10U, "record_count should be parsed");
    expect(result.header.header_length == 161U, "header_length should be parsed");
    expect(result.header.record_length == 64U, "record_length should be parsed");
    expect(result.header.has_structural_cdx(), "structural CDX flag should be detected");
    expect(result.header.has_database_container(), "database container flag should be detected");
    expect(result.header.format_family() == copperfin::vfp::DbfFormatFamily::visual_foxpro,
           "VFP version byte should classify as the visual_foxpro inspection family");
    expect(std::string(copperfin::vfp::dbf_format_family_name(result.header.format_family())) == "visual_foxpro",
           "VFP inspection family should use the stable machine-readable name");
    expect(result.header.version_description() == "Visual FoxPro", "version description should match Visual FoxPro");
    expect(result.header.last_update_iso8601() == "2026-04-07", "last update date should be formatted as ISO 8601");

    copperfin::vfp::DbfHeader dbase_header;
    dbase_header.version = 0x8BU;
    expect(dbase_header.format_family() == copperfin::vfp::DbfFormatFamily::dbase,
           "dBASE memo version byte should classify as the dbase inspection family");
    dbase_header.version = 0x04U;
    expect(dbase_header.format_family() == copperfin::vfp::DbfFormatFamily::dbase,
           "dBASE Level 7 version byte should classify as the dbase inspection family");
    dbase_header.version = 0x0CU;
    expect(dbase_header.format_family() == copperfin::vfp::DbfFormatFamily::dbase,
        "dBASE Level 7 memo-flag version byte should retain the dbase inspection family");
    dbase_header.version = 0x8CU;
    expect(dbase_header.has_memo_file(),
        "dBASE Level 7 bit-7 memo flag should report its DBT sidecar");
    expect(dbase_header.version_description() == "dBASE Level 7 with memo",
        "dBASE Level 7 memo flags should have a precise localized description");
    dbase_header.version = 0x1CU;
    expect(dbase_header.version_description() == "dBASE Level 7 SQL table with memo",
        "dBASE Level 7 SQL and memo flags should compose in the description");
    copperfin::vfp::DbfHeader foxbase_header;
    foxbase_header.version = 0x02U;
    expect(foxbase_header.format_family() == copperfin::vfp::DbfFormatFamily::foxbase,
           "FoxBase version byte should classify as the foxbase inspection family");
    foxbase_header.version = 0xFBU;
    expect(foxbase_header.format_family() == copperfin::vfp::DbfFormatFamily::foxbase,
           "#5518: 0xfb is a documented FoxBASE signature, not FoxPro, and should classify as foxbase");
    expect(!foxbase_header.has_memo_file(), "#5518: 0xfb (FoxBASE) never carries a memo file");
    expect(foxbase_header.version_description() == "FoxBASE",
           "#5518: 0xfb should report the same version description as 0x02");
    copperfin::vfp::DbfHeader unknown_header;
    unknown_header.version = 0x7FU;
    expect(unknown_header.format_family() == copperfin::vfp::DbfFormatFamily::unknown,
           "unrecognized version bytes should remain explicitly unknown");
}

void test_last_update_iso8601_applies_fixed_century_rollover_for_real_foxbase_byte() {
    // #5527: real FoxBASE+ 2.10 (not an emulator or reconstruction) wrote
    // header byte 1 = 0x1A (26 decimal) for a table last updated
    // 2026-09-09 -- the genuine dBASE-family on-disk convention is a
    // two-digit calendar year (year % 100), not years-since-1900, despite
    // the nominal DBF format documentation's 1900-2155 framing. Before
    // this fix, last_update_iso8601() misread this as 1926.
    copperfin::vfp::DbfHeader real_foxbase_header;
    real_foxbase_header.last_update_year = 26U;
    real_foxbase_header.last_update_month = 9U;
    real_foxbase_header.last_update_day = 9U;
    expect(real_foxbase_header.last_update_iso8601() == "2026-09-09",
           "#5527: a real FoxBASE+ 2.10 two-digit-year byte (26) should resolve to 2026, not 1926");

    // A byte >= 100 is unambiguous (it cannot be a genuine two-digit
    // year) and must still mean 1900 + byte, matching this function's
    // behavior before #5527 -- this is how Copperfin's own writer
    // stamped dates prior to this fix, and how any byte in that range
    // must continue to be read for files it already wrote.
    copperfin::vfp::DbfHeader legacy_unbounded_header;
    legacy_unbounded_header.last_update_year = 126U;
    legacy_unbounded_header.last_update_month = 4U;
    legacy_unbounded_header.last_update_day = 7U;
    expect(legacy_unbounded_header.last_update_iso8601() == "2026-04-07",
           "#5527: a byte >= 100 is unambiguous and should always mean 1900 + byte");

    // A byte at or above the fixed rollover threshold (80) -- e.g. a
    // real 1991 FoxBASE fixture, byte 91 -- must resolve to the 1900s,
    // and must keep doing so indefinitely: this is exactly why the
    // threshold is fixed rather than "now"-relative (see
    // last_update_iso8601()'s own comment) -- a real 1980s/1990s legacy
    // byte should never start being misread as 2080s/2090s just because
    // enough calendar time has passed since this test was written.
    copperfin::vfp::DbfHeader old_legacy_header;
    old_legacy_header.last_update_year = 91U;
    old_legacy_header.last_update_month = 6U;
    old_legacy_header.last_update_day = 15U;
    expect(old_legacy_header.last_update_iso8601() == "1991-06-15",
           "#5527: a two-digit year at or above the fixed rollover threshold should resolve to the 1900s, not the 2000s");
}

void test_parse_dbf_header_rejects_short_input() {
    const auto result = copperfin::vfp::parse_dbf_header({0x30U, 0x00U});
    expect(!result.ok, "parse_dbf_header should reject short input");
    expect(
        result.error == "File is smaller than the minimum DBF header size (32 bytes).",
        "#2379: parse_dbf_header should preserve the default localized short-header error");
}

void test_dbf_cdx_header_errors_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.DbfHeader.Error.ShortHeader") ==
            "File is smaller than the minimum DBF header size (32 bytes).",
        "#2379: DBF header short-input error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfHeader.Version.VisualFoxPro") == "Visual FoxPro",
        "#2494: DBF header version descriptions should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.CdxHeader.Error.InvalidValues") ==
            "Header values do not look like a CDX-family index file.",
        "#2379: CDX invalid-header error should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate("Vfp.DbfHeader.Error.ShortHeader") ==
            "El archivo es menor que el tamano minimo del encabezado DBF (32 bytes).",
        "#2602: DBF header short-input error should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Vfp.CdxHeader.Error.InvalidValues") ==
            "Os valores do cabecalho nao parecem pertencer a um arquivo de indice da familia CDX.",
        "#2602: CDX invalid-header error should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Vfp.DbfHeader.Error.ShortHeader") !=
            english_catalog.translate("Vfp.DbfHeader.Error.ShortHeader"),
        "#2379: DBF header errors should be pseudo-localizable");
    expect(
        pseudo_catalog.translate("Vfp.DbfHeader.Error.ShortHeader") ==
            copperfin::localization::pseudo_localize(
                "File is smaller than the minimum DBF header size (32 bytes)."),
        "#2602: DBF header pseudo-locale should route through the pseudo-localization transform");

    const auto parsed_result = copperfin::vfp::parse_dbf_header(make_vfp_header());
    expect(parsed_result.ok, "#2494: DBF header parse should remain valid before version localization checks");
    expect(parsed_result.header.version == 0x30U, "#2494: DBF header version byte should remain invariant");
    expect(
        parsed_result.header.version_description(english_catalog) == "Visual FoxPro",
        "#2494: DBF header version description should preserve default en-US prose");
    expect(
        parsed_result.header.version_description(pseudo_catalog).find("[!! ") != std::string::npos,
        "#2494: DBF header version description should route through pseudo-localization");
    expect(
        parsed_result.header.version_description(pseudo_catalog).find("Visual FoxPro") == std::string::npos,
        "#2494: pseudo-localized DBF header version description should not fall back to raw English prose");

    copperfin::vfp::DbfHeader unknown_header;
    unknown_header.version = 0x7FU;
    expect(
        unknown_header.version_description(english_catalog) == "Unknown",
        "#2494: unknown DBF header version description should preserve default en-US prose");
    expect(
        unknown_header.version_description(pseudo_catalog).find("[!! ") != std::string::npos,
        "#2494: unknown DBF header version description should route through pseudo-localization");

    const auto cdx_result = copperfin::vfp::parse_cdx_header({0x00U, 0x04U}, 2U);
    expect(!cdx_result.ok, "parse_cdx_header should reject short input");
    expect(
        cdx_result.error == "File is smaller than the minimum CDX header probe size (16 bytes).",
        "#2379: parse_cdx_header should preserve the default localized short-probe error");
}

void test_vfp_header_and_index_default_catalog_refresh() {
    const copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");

    const auto english_dbf = copperfin::vfp::parse_dbf_header({0x30U, 0x00U});
    const auto english_cdx = copperfin::vfp::parse_cdx_header({0x00U, 0x04U}, 2U);
    const auto english_idx = copperfin::vfp::parse_index_probe(
        {0x00U},
        1U,
        copperfin::vfp::IndexKind::idx);
    expect(english_dbf.error.find("File is smaller") != std::string::npos,
           "#4358: DBF header default diagnostic should begin in en-US");
    expect(english_cdx.error.find("File is smaller") != std::string::npos,
           "#4358: CDX header default diagnostic should begin in en-US");
    expect(english_idx.error.find("File is smaller") != std::string::npos,
           "#4358: index probe default diagnostic should begin in en-US");

    locale.set("es-419");
    const auto spanish_dbf = copperfin::vfp::parse_dbf_header({0x30U, 0x00U});
    const auto spanish_cdx = copperfin::vfp::parse_cdx_header({0x00U, 0x04U}, 2U);
    const auto spanish_idx = copperfin::vfp::parse_index_probe(
        {0x00U},
        1U,
        copperfin::vfp::IndexKind::idx);
    expect(spanish_dbf.error != english_dbf.error,
           "#4358: DBF header diagnostics should refresh to es-419");
    expect(spanish_cdx.error != english_cdx.error,
           "#4358: CDX header diagnostics should refresh to es-419");
    expect(spanish_idx.error != english_idx.error,
           "#4358: index probe diagnostics should refresh to es-419");

    locale.set("qps-ploc");
    const auto pseudo_dbf = copperfin::vfp::parse_dbf_header({0x30U, 0x00U});
    const auto pseudo_cdx = copperfin::vfp::parse_cdx_header({0x00U, 0x04U}, 2U);
    const auto pseudo_idx = copperfin::vfp::parse_index_probe(
        {0x00U},
        1U,
        copperfin::vfp::IndexKind::idx);
    expect(pseudo_dbf.error.find("[!! ") != std::string::npos,
           "#4358: DBF header diagnostics should refresh to qps-ploc");
    expect(pseudo_cdx.error.find("[!! ") != std::string::npos,
           "#4358: CDX header diagnostics should refresh to qps-ploc");
    expect(pseudo_idx.error.find("[!! ") != std::string::npos,
           "#4358: index probe diagnostics should refresh to qps-ploc");
}

void test_asset_family_detection() {
    using copperfin::vfp::AssetFamily;
    using copperfin::vfp::asset_family_from_path;

    expect(asset_family_from_path("sample.pjx") == AssetFamily::project, "PJX should map to project");
    expect(asset_family_from_path("sample.scx") == AssetFamily::form, "SCX should map to form");
    expect(asset_family_from_path("sample.vcx") == AssetFamily::class_library, "VCX should map to class library");
    expect(asset_family_from_path("sample.frx") == AssetFamily::report, "FRX should map to report");
    expect(asset_family_from_path("sample.lbx") == AssetFamily::label, "LBX should map to label");
    expect(asset_family_from_path("sample.mnx") == AssetFamily::menu, "MNX should map to menu");
    expect(asset_family_from_path("sample.cdx") == AssetFamily::index, "CDX should map to index");
    expect(asset_family_from_path("sample.idx") == AssetFamily::index, "IDX should map to index");
    expect(asset_family_from_path("sample.ndx") == AssetFamily::index, "NDX should map to index");
    expect(asset_family_from_path("sample.mdx") == AssetFamily::index, "MDX should map to index");
    expect(asset_family_from_path("sample.ntx") == AssetFamily::index, "NTX should map to index");
    expect(asset_family_from_path("sample.prg") == AssetFamily::program, "PRG should map to program");
    expect(asset_family_from_path("sample.h") == AssetFamily::header, "H should map to header");
    expect(asset_family_from_path("sample.xyz") == AssetFamily::unknown, "unknown extension should stay unknown");
}

void test_asset_inspector_errors_resolve_through_localization_catalog() {
    namespace fs = std::filesystem;
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap path_placeholder{{"path", "missing.dbc"}};
    const copperfin::localization::PlaceholderMap error_placeholder{{"error", "inner parse failure"}};

    expect(
        english_catalog.translate("Vfp.AssetInspector.Error.PathMissing") == "Path does not exist.",
        "#2386: asset inspector missing-path error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.AssetInspector.Error.ReadFailed", path_placeholder) ==
            "Unable to read asset: missing.dbc",
        "#4354: asset inspector read failures should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.AssetInspector.Error.DbcPathMissing", path_placeholder) ==
            "DBC path does not exist: missing.dbc",
        "#2386: DBC export missing-path error should preserve named placeholders");
    expect(
        pseudo_catalog.translate("Vfp.AssetInspector.Error.PathMissing") !=
            english_catalog.translate("Vfp.AssetInspector.Error.PathMissing"),
        "#2386: asset inspector errors should be pseudo-localizable");
    expect(
        english_catalog.translate("Vfp.AssetInspector.Validation.DbfHeaderLengthExceedsFileSize") ==
            "The DBF header length exceeds the file size.",
        "#2387: DBF validation messages should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.AssetInspector.Validation.IndexCompanionParseFailed", error_placeholder) ==
            "A companion index file exists but could not be parsed: inner parse failure",
        "#2387: asset inspector validation messages should preserve named placeholders");
    expect(
        spanish_catalog.translate("Vfp.AssetInspector.Error.DbcPathMissing", path_placeholder) ==
            "La ruta DBC no existe: missing.dbc",
        "#2602: asset inspector DBC path errors should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Vfp.AssetInspector.Validation.IndexCompanionParseFailed", error_placeholder) ==
            "Existe um arquivo de indice complementar, mas nao foi possivel analisa-lo: inner parse failure",
        "#2602: asset inspector validation placeholders should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Vfp.AssetInspector.Validation.MemoSidecarMissing") !=
            english_catalog.translate("Vfp.AssetInspector.Validation.MemoSidecarMissing"),
        "#2387: asset inspector validation messages should be pseudo-localizable");
    expect(
        pseudo_catalog.translate("Vfp.AssetInspector.Error.PathMissing") ==
            copperfin::localization::pseudo_localize("Path does not exist."),
        "#2602: asset inspector qps-ploc strings should resolve through the pseudo-localization transform");

    const fs::path temp_path = fs::temp_directory_path() / "copperfin_missing_asset_for_localization.dbc";
    std::error_code ignored;
    fs::remove(temp_path, ignored);

    const auto inspect_result = copperfin::vfp::inspect_asset(temp_path.string());
    expect(!inspect_result.ok, "#2386: inspect_asset should reject missing paths");
    expect(
        inspect_result.error == "Path does not exist.",
        "#2386: inspect_asset should preserve the default localized missing-path error");

    const auto export_result = copperfin::vfp::export_database_as_json(temp_path.string(), 10U);
    expect(!export_result.ok, "#2386: export_database_as_json should reject missing DBC paths");
    expect(
        export_result.error == "DBC path does not exist: " + temp_path.string(),
        "#2386: export_database_as_json should preserve the default localized missing-DBC error");
    expect(export_result.json.empty(),
           "#3988: failed database exports should leave the JSON result empty");

    const auto sql_export_result = copperfin::vfp::export_database_as_sql(temp_path.string(), 10U);
    expect(!sql_export_result.ok, "#5471: export_database_as_sql should reject missing DBC paths");
    expect(
        sql_export_result.error == "DBC path does not exist: " + temp_path.string(),
        "#5471: export_database_as_sql should share export_database_as_json's default localized missing-DBC error");
    expect(sql_export_result.sql.empty(),
           "#5471: failed SQL database exports should leave the SQL result empty");

    const auto access_export_result = copperfin::vfp::export_database_as_access_sql(temp_path.string(), 10U);
    expect(!access_export_result.ok, "#5475: export_database_as_access_sql should reject missing DBC paths");
    expect(
        access_export_result.error == "DBC path does not exist: " + temp_path.string(),
        "#5475: export_database_as_access_sql should share export_database_as_json's default localized missing-DBC error");
    expect(access_export_result.sql.empty(),
           "#5475: failed Access SQL database exports should leave the SQL result empty");

    copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    const auto english_inspect_result = copperfin::vfp::inspect_asset(temp_path.string());
    locale.set("es-419");
    const auto spanish_inspect_result = copperfin::vfp::inspect_asset(temp_path.string());
    locale.set("qps-ploc");
    const auto pseudo_inspect_result = copperfin::vfp::inspect_asset(temp_path.string());
    expect(
        english_inspect_result.error == "Path does not exist." &&
            spanish_inspect_result.error == "La ruta no existe." &&
            pseudo_inspect_result.error == copperfin::localization::pseudo_localize("Path does not exist."),
        "#4355: asset inspector diagnostics should refresh after in-process locale changes");
}

#if !defined(_WIN32)
void test_inspect_asset_inaccessible_path_returns_structured_failure() {
    namespace fs = std::filesystem;
    const fs::path temp_dir =
        fs::temp_directory_path() / "copperfin_vfp_asset_inaccessible_path_tests";
    const fs::path restricted_dir = temp_dir / "restricted";
    const fs::path asset_path = restricted_dir / "blocked.dbf";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(restricted_dir);

    {
        const auto bytes = make_vfp_header();
        std::ofstream output(asset_path, std::ios::binary);
        output.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }

    std::error_code status_error;
    const fs::perms original_permissions = fs::status(restricted_dir, status_error).permissions();
    expect(
        !status_error,
        "#4354: inaccessible asset fixture should report its original directory permissions");
    if (status_error) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    fs::permissions(
        restricted_dir,
        fs::perms::none,
        fs::perm_options::replace,
        status_error);
    expect(
        !status_error,
        "#4354: inaccessible asset fixture should remove directory permissions");
    if (status_error) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    std::ifstream permission_probe(asset_path, std::ios::binary);
    const bool access_is_denied = !permission_probe.good();
    expect(
        access_is_denied,
        "#4354: inaccessible asset fixture should be unreadable when the host enforces POSIX permissions");
    permission_probe.close();

    if (access_is_denied) {
        const auto result = copperfin::vfp::inspect_asset(asset_path.string());
        expect(
            !result.ok,
            "#4354: inaccessible asset inspection should return a structured failure instead of throwing");
        expect(
            result.error == "Path does not exist.",
            "#4354: inaccessible asset inspection should preserve the existing missing-path contract");
    }

    fs::permissions(
        restricted_dir,
        original_permissions,
        fs::perm_options::replace,
        ignored);
    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_json_inaccessible_path_returns_structured_failure() {
    namespace fs = std::filesystem;
    const fs::path temp_dir =
        fs::temp_directory_path() / "copperfin_vfp_dbc_inaccessible_path_tests";
    const fs::path restricted_dir = temp_dir / "restricted";
    const fs::path dbc_path = restricted_dir / "blocked.dbc";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(restricted_dir);

    std::error_code status_error;
    const fs::perms original_permissions = fs::status(restricted_dir, status_error).permissions();
    expect(
        !status_error,
        "#4405: inaccessible DBC fixture should report its original directory permissions");
    if (status_error) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    fs::permissions(
        restricted_dir,
        fs::perms::none,
        fs::perm_options::replace,
        status_error);
    expect(
        !status_error,
        "#4405: inaccessible DBC fixture should remove directory permissions");
    if (status_error) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    std::error_code probe_error;
    const bool access_is_denied = !fs::exists(dbc_path, probe_error) && static_cast<bool>(probe_error);
    expect(
        access_is_denied,
        "#4405: inaccessible DBC fixture should produce a filesystem status error");

    if (access_is_denied) {
        const auto result = copperfin::vfp::export_database_as_json(dbc_path.string());
        expect(
            !result.ok,
            "#4405: inaccessible DBC export should return a structured failure instead of throwing");
        expect(
            result.error == "DBC path does not exist: " + dbc_path.string(),
            "#4405: inaccessible DBC export should preserve the missing-path contract");
        expect(
            result.json.empty(),
            "#4405: inaccessible DBC export should leave the JSON result empty");
    }

    fs::permissions(
        restricted_dir,
        original_permissions,
        fs::perm_options::replace,
        ignored);
    fs::remove_all(temp_dir, ignored);
}
#endif

#include "test_vfp_assets_index_probe_contracts.inl"

void test_inspect_asset_collects_companion_indexes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string("copperfin_vfp_assets_\xC3\xA9_tests");
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / copperfin::platform::path_from_utf8_string("caf\xC3\xA9.dbf");
    const fs::path cdx_path = temp_dir / copperfin::platform::path_from_utf8_string("caf\xC3\xA9.cdx");
    const fs::path ndx_path = temp_dir / copperfin::platform::path_from_utf8_string("caf\xC3\xA9.ndx");
    const fs::path mdx_path = temp_dir / copperfin::platform::path_from_utf8_string("caf\xC3\xA9.mdx");
    const fs::path ntx_path = temp_dir / copperfin::platform::path_from_utf8_string("caf\xC3\xA9.ntx");

    {
        auto bytes = make_vfp_header();
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        const auto bytes = make_synthetic_cdx_family_bytes(false, true);
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::vector<std::uint8_t> bytes(1024U, 0U);
        write_le_u32(bytes, 0U, 1U);
        write_le_u32(bytes, 4U, 2U);
        write_le_u16(bytes, 12U, 2U);
        write_le_u16(bytes, 14U, 42U);
        write_le_u16(bytes, 18U, 12U);
        write_ascii(bytes, 24U, "CODE");

        std::ofstream output(ndx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        const auto bytes = make_synthetic_mdx_bytes(true);
        std::ofstream output(mdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::vector<std::uint8_t> bytes(1024U, 0U);
        write_le_u16(bytes, 0U, 0x0006U);
        write_le_u32(bytes, 4U, 1024U);
        write_le_u16(bytes, 12U, 18U);
        write_le_u16(bytes, 14U, 10U);
        write_le_u16(bytes, 18U, 46U);
        write_ascii(bytes, 22U, "CODE");

        std::ofstream output(ntx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        // Pad to a full second 1024-byte page so the file size is itself a
        // whole number of pages, matching the root-offset page it claims.
        std::vector<std::uint8_t> padding(1024U, 0U);
        output.write(reinterpret_cast<const char*>(padding.data()), static_cast<std::streamsize>(padding.size()));
    }

    const auto result = copperfin::vfp::inspect_asset(
        copperfin::platform::path_to_utf8_string(table_path));
    expect(result.ok, "inspect_asset should succeed for a synthetic DBF with companion indexes");
    expect(result.header_available, "inspect_asset should expose the DBF header");
    expect(result.indexes.size() == 4U, "inspect_asset should collect same-base CDX, NDX, MDX, and NTX companions");

    bool saw_cdx = false;
    bool saw_ndx = false;
    bool saw_mdx = false;
    bool saw_ntx = false;
    for (const auto& index : result.indexes) {
        saw_cdx = saw_cdx || index.probe.kind == copperfin::vfp::IndexKind::cdx;
        saw_ndx = saw_ndx || index.probe.kind == copperfin::vfp::IndexKind::ndx;
        saw_mdx = saw_mdx || index.probe.kind == copperfin::vfp::IndexKind::mdx;
        saw_ntx = saw_ntx || index.probe.kind == copperfin::vfp::IndexKind::ntx;
        if (index.probe.kind == copperfin::vfp::IndexKind::cdx) {
            expect(!index.probe.tags.empty(), "inspect_asset should parse CDX companion tags");
            if (!index.probe.tags.empty()) {
                expect(index.probe.tags.front().name_hint == "NAME", "inspect_asset should expose the real CDX tag name");
                expect(index.probe.tags.front().key_expression_hint == "UPPER(NAME)", "inspect_asset should expose the CDX expression hint");
                expect(index.probe.tags.front().for_expression_hint == "DELETED() = .F.", "inspect_asset should expose the CDX FOR expression hint");
            }
        }
        if (index.probe.kind == copperfin::vfp::IndexKind::mdx) {
            expect(index.probe.tags.size() == 2U, "inspect_asset should expose first-pass MDX tag hints");
            if (index.probe.tags.size() >= 2U) {
                expect(index.probe.tags[0].name_hint == "NAME_TAG", "inspect_asset should expose the first MDX tag hint");
                expect(index.probe.tags[1].name_hint == "CITYSTATE", "inspect_asset should expose the second MDX tag hint");
                expect(index.probe.tags[0].name_hint != "LATE_BLOCK_TEXT", "inspect_asset should not promote late block text into MDX tags");
                expect(index.probe.tags[0].key_expression_hint == "UPPER(NAME)", "inspect_asset should expose first-pass MDX key-expression metadata");
                expect(index.probe.tags[0].for_expression_hint == "DELETED() = .F.", "inspect_asset should expose first-pass MDX FOR-expression metadata");
            }
        }
    }

    expect(saw_cdx, "inspect_asset should identify CDX companions");
    expect(saw_ndx, "inspect_asset should identify NDX companions");
    expect(saw_mdx, "inspect_asset should identify MDX companions");
    expect(saw_ntx, "inspect_asset should identify NTX companions");

    std::error_code ignored;
    fs::remove(table_path, ignored);
    fs::remove(cdx_path, ignored);
    fs::remove(ndx_path, ignored);
    fs::remove(mdx_path, ignored);
    fs::remove(ntx_path, ignored);
    fs::remove(temp_dir, ignored);
}

void test_inspect_asset_uses_admitted_index_bytes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_verified_index_inspection_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path index_path = temp_dir / "sample.cdx";
    const auto admitted_bytes = make_synthetic_cdx_family_bytes(false, true);
    const auto mutated_bytes = make_synthetic_cdx_family_bytes(true, true);
    {
        std::ofstream output(index_path, std::ios::binary);
        output.write(
            reinterpret_cast<const char*>(mutated_bytes.data()),
            static_cast<std::streamsize>(mutated_bytes.size()));
    }

    copperfin::vfp::AssetByteOverrides overrides{
        {index_path.lexically_normal().string(),
         std::string(admitted_bytes.begin(), admitted_bytes.end())}};
    const auto result = copperfin::vfp::inspect_asset(index_path.string(), {}, &overrides);
    expect(result.ok, "verified index inspection should parse admitted bytes");
    expect(result.indexes.size() == 1U, "verified index inspection should return one index asset");
    if (result.indexes.size() == 1U) {
        expect(
            result.indexes.front().probe.tags.size() == 1U,
            "verified index inspection should ignore a post-admission pathname mutation");
        expect(
            result.indexes.front().probe.file_size == admitted_bytes.size(),
            "verified index inspection should report the admitted byte size");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_inspect_asset_discovers_virtual_casefolded_index_bytes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_virtual_index_inspection_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "sample.dbf";
    const auto created = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 16U}},
        {{"Ada"}});
    expect(created.ok, "virtual index inspection should create its DBF fixture");

    const fs::path differently_cased_index_path = temp_dir / "SAMPLE.CDX";
    const auto admitted_bytes = make_synthetic_cdx_family_bytes(false, true);
    copperfin::vfp::AssetByteOverrides overrides{
        {differently_cased_index_path.lexically_normal().string(),
         std::string(admitted_bytes.begin(), admitted_bytes.end())}};
    const auto result = copperfin::vfp::inspect_asset(table_path.string(), {}, &overrides);
    expect(result.ok, "virtual index inspection should accept a valid DBF without a physical companion");
#if defined(_WIN32)
    expect(result.indexes.size() == 1U,
           "Windows virtual index inspection should retain VFP case-insensitive companion admission");
#else
    expect(result.indexes.empty(),
           "POSIX virtual index inspection should reject a differently-cased companion override");
#endif

    fs::remove_all(temp_dir, ignored);
}

void test_inspect_database_container_collects_dcx_companion() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_dbc_assets_tests";
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "sample.dbc";
    const fs::path dcx_path = temp_dir / "sample.dcx";

    {
        auto bytes = make_vfp_header();
        std::ofstream output(dbc_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        const auto bytes = make_synthetic_cdx_family_bytes(false, true);
        std::ofstream output(dcx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto result = copperfin::vfp::inspect_asset(dbc_path.string());
    expect(result.ok, "inspect_asset should succeed for a synthetic DBC with a companion DCX");
    expect(result.header_available, "inspect_asset should expose the DBC header");
    expect(result.indexes.size() == 1U, "inspect_asset should collect the same-base DCX companion");
    if (result.indexes.size() == 1U) {
        expect(result.indexes.front().probe.kind == copperfin::vfp::IndexKind::dcx, "DBC companion probe should stay typed as DCX");
        expect(!result.indexes.front().probe.tags.empty(), "DBC companion inspection should parse DCX tags");
        if (!result.indexes.front().probe.tags.empty()) {
            expect(result.indexes.front().probe.tags.front().name_hint == "NAME", "DBC companion inspection should expose the DCX tag name");
            expect(result.indexes.front().probe.tags.front().key_expression_hint == "UPPER(NAME)", "DBC companion inspection should expose the DCX key expression");
        }
    }

    std::error_code ignored;
    fs::remove(dbc_path, ignored);
    fs::remove(dcx_path, ignored);
    fs::remove(temp_dir, ignored);
}

bool write_binary_file(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        return false;
    }
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(output);
}

std::vector<std::uint8_t> make_access_container_bytes(std::string_view signature, std::uint8_t generation_byte) {
    std::vector<std::uint8_t> bytes(64U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x01U;
    bytes[2] = 0x00U;
    bytes[3] = 0x00U;
    write_ascii(bytes, 4U, std::string(signature));
    bytes[0x14U] = generation_byte;
    return bytes;
}

void test_parse_access_container_header_for_jet3_mdb() {
    const auto bytes = make_access_container_bytes("Standard Jet DB", 0x00U);

    const auto result = copperfin::vfp::parse_access_container_header(bytes);
    expect(result.ok, "parse_access_container_header should succeed for a plausible Jet 3 MDB header");
    expect(result.header.family == copperfin::vfp::AccessContainerFamily::jet, "Jet signature should classify as the jet family");
    expect(result.header.generation == copperfin::vfp::AccessContainerGeneration::jet3, "generation byte 0x00 should classify as jet3");
    expect(result.header.looks_like_access_container(), "a recognized family should look like an Access container");
}

void test_parse_access_container_header_for_jet4_mdb() {
    const auto bytes = make_access_container_bytes("Standard Jet DB", 0x01U);

    const auto result = copperfin::vfp::parse_access_container_header(bytes);
    expect(result.ok, "parse_access_container_header should succeed for a plausible Jet 4 MDB header");
    expect(result.header.generation == copperfin::vfp::AccessContainerGeneration::jet4, "generation byte 0x01 should classify as jet4");
}

void test_parse_access_container_header_for_ace_accdb() {
    const auto bytes = make_access_container_bytes("Standard ACE DB", 0x02U);

    const auto result = copperfin::vfp::parse_access_container_header(bytes);
    expect(result.ok, "parse_access_container_header should succeed for a plausible ACE ACCDB header");
    expect(result.header.family == copperfin::vfp::AccessContainerFamily::ace, "ACE signature should classify as the ace family");
    expect(result.header.generation == copperfin::vfp::AccessContainerGeneration::later, "generation byte 0x02 should classify only as the coarse 'later' bucket, not a specific edition");
    expect(result.header.generation_byte == 0x02U, "the raw generation byte should be preserved even when the coarse bucket is 'later'");
}

void test_parse_access_container_header_rejects_missing_signature() {
    std::vector<std::uint8_t> bytes(64U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x01U;
    bytes[2] = 0x00U;
    bytes[3] = 0x00U;
    write_ascii(bytes, 4U, "Not A Real Signature");

    const auto result = copperfin::vfp::parse_access_container_header(bytes);
    expect(!result.ok, "parse_access_container_header should reject a file without a recognized Jet/ACE signature");
}

void test_parse_access_container_header_rejects_truncated_file() {
    std::vector<std::uint8_t> bytes(10U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x01U;
    bytes[2] = 0x00U;
    bytes[3] = 0x00U;

    const auto result = copperfin::vfp::parse_access_container_header(bytes);
    expect(!result.ok, "parse_access_container_header should reject a file truncated before the generation byte");
}

void test_access_container_errors_resolve_through_localization_catalog() {
    const copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.AccessContainer.Error.SignatureMismatch") ==
            "Header bytes do not look like a Jet or ACE database container.",
        "#5521: Access container signature-mismatch error should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate("Vfp.AccessContainer.Error.SignatureMismatch") ==
            "Los bytes del encabezado no parecen corresponder a un contenedor de base de datos Jet o ACE.",
        "#5521: Access container signature-mismatch error should resolve through the es-419 catalog");
    expect(
        pseudo_catalog.translate("Vfp.AccessContainer.Error.SignatureMismatch") !=
            english_catalog.translate("Vfp.AccessContainer.Error.SignatureMismatch"),
        "#5521: Access container errors should be pseudo-localizable");

    const auto short_result = copperfin::vfp::parse_access_container_header({0x00U});
    expect(!short_result.ok, "parse_access_container_header should reject a near-empty buffer");
    expect(
        short_result.error == "File is smaller than the minimum Access container header probe size (21 bytes).",
        "#5521: parse_access_container_header should preserve the default localized short-header error");
}

// #5476: synthetic Jet3/Jet4 Table Definition (TDEF) page builders,
// reproducing the byte layout documented in
// docs/68-access-mdb-jet-physical-page-layout-notes.md and independently
// cross-checked against real Jet3/Jet4 .mdb fixtures during this slice's
// development (see that header's own comment for the specifics). Building
// these from the verified layout, rather than reusing an actual fixture
// file, keeps the committed test suite free of any real Access file --
// real fixtures available locally include ones the user has said contain
// genuine personal data, and this project's practice is not to commit
// such files or their content regardless of which specific file a test
// happens to need.
struct SyntheticAccessColumn {
    std::string name;
    std::uint8_t type = 0U;
    std::uint16_t length = 0U;
    bool fixed = true;
    // #5539: overrides the column-descriptor's own col_num field, which
    // is normally just this column's position in `columns` (matching
    // #5476's original synthetic builders). A real Jet4 fixture found
    // during #5539's development stored its MSysObjects column
    // descriptors in a PHYSICAL order that did not match their logical
    // col_num (alphabetized by name rather than declaration order) --
    // this override lets a test reproduce that same physical/logical
    // mismatch rather than only ever testing the case where they
    // coincide.
    std::optional<std::uint16_t> column_number_override;
};

// Real Access assigns each fixed column's offset_F sequentially in
// col_num order, independent of the column descriptors' own physical
// storage order in the TDEF page (confirmed during #5539's real-fixture
// cross-validation -- see SyntheticAccessColumn::column_number_override's
// own comment). Returns, for each entry in `columns` (by its own vector
// position), the offset_F a fixed column should use; the value for a
// non-fixed column is unspecified (never read).
std::vector<std::uint16_t> compute_synthetic_offset_f_by_col_num_order(
    const std::vector<SyntheticAccessColumn>& columns) {
    std::vector<std::size_t> order(columns.size());
    for (std::size_t index = 0U; index < columns.size(); ++index) {
        order[index] = index;
    }
    std::sort(order.begin(), order.end(), [&](std::size_t left, std::size_t right) {
        return columns[left].column_number_override.value_or(static_cast<std::uint16_t>(left)) <
               columns[right].column_number_override.value_or(static_cast<std::uint16_t>(right));
    });
    std::vector<std::uint16_t> offset_f(columns.size(), 0U);
    std::uint16_t cursor = 0U;
    for (const std::size_t index : order) {
        if (columns[index].fixed) {
            offset_f[index] = cursor;
            cursor = static_cast<std::uint16_t>(cursor + columns[index].length);
        }
    }
    return offset_f;
}

std::vector<std::uint8_t> make_synthetic_jet3_tdef_page(
    const std::vector<SyntheticAccessColumn>& columns,
    std::uint8_t table_type,
    std::uint32_t num_rows) {
    std::vector<std::uint8_t> page(2048U, 0U);
    page[0] = 0x02U;
    page[1] = 0x01U;
    page[2] = 'V';
    page[3] = 'C';
    // next_pg (bytes 4-7) stays 0: a single-page TDEF.

    std::size_t offset = 8U;
    const auto num_cols = static_cast<std::uint16_t>(columns.size());
    write_le_u32(page, offset, 0U); offset += 4U;  // tdef_len (not validated by the parser)
    write_le_u32(page, offset, num_rows); offset += 4U;
    write_le_u32(page, offset, 0U); offset += 4U;  // autonumber
    page[offset] = table_type; offset += 1U;
    write_le_u16(page, offset, num_cols); offset += 2U;  // max_cols
    std::uint16_t num_var_cols = 0U;
    for (const auto& column : columns) {
        if (!column.fixed) {
            ++num_var_cols;
        }
    }
    write_le_u16(page, offset, num_var_cols); offset += 2U;
    write_le_u16(page, offset, num_cols); offset += 2U;  // num_cols
    write_le_u32(page, offset, 0U); offset += 4U;  // num_idx
    write_le_u32(page, offset, 0U); offset += 4U;  // num_real_idx = 0 (no index-info block to skip)
    write_le_u32(page, offset, 0U); offset += 4U;  // used_pages
    write_le_u32(page, offset, 0U); offset += 4U;  // free_pages

    const std::vector<std::uint16_t> offset_f_table = compute_synthetic_offset_f_by_col_num_order(columns);
    for (std::size_t index = 0U; index < columns.size(); ++index) {
        const auto& column = columns[index];
        const auto col_num = column.column_number_override.value_or(static_cast<std::uint16_t>(index));
        page[offset] = column.type; offset += 1U;
        write_le_u16(page, offset, col_num); offset += 2U;  // col_num
        write_le_u16(page, offset, 0U); offset += 2U;  // offset_V
        write_le_u16(page, offset, col_num); offset += 2U;  // col_num (repeat)
        write_le_u16(page, offset, 0x409U); offset += 2U;  // sort_order
        write_le_u16(page, offset, 0U); offset += 2U;  // misc
        write_le_u16(page, offset, 0U); offset += 2U;  // unknown
        page[offset] = column.fixed ? 0x01U : 0x00U; offset += 1U;  // bitmask
        write_le_u16(page, offset, offset_f_table[index]); offset += 2U;  // offset_F
        write_le_u16(page, offset, column.length); offset += 2U;  // col_len
    }
    for (const auto& column : columns) {
        page[offset] = static_cast<std::uint8_t>(column.name.size()); offset += 1U;
        std::copy(column.name.begin(), column.name.end(), page.begin() + static_cast<std::ptrdiff_t>(offset));
        offset += column.name.size();
    }
    return page;
}

std::vector<std::uint8_t> make_synthetic_jet4_tdef_page(
    const std::vector<SyntheticAccessColumn>& columns,
    std::uint8_t table_type,
    std::uint32_t num_rows) {
    std::vector<std::uint8_t> page(4096U, 0U);
    page[0] = 0x02U;
    page[1] = 0x01U;
    // next_pg (bytes 4-7) stays 0: a single-page TDEF.

    std::size_t offset = 8U;
    const auto num_cols = static_cast<std::uint16_t>(columns.size());
    write_le_u32(page, offset, 0U); offset += 4U;  // tdef_len
    write_le_u32(page, offset, 0U); offset += 4U;  // unknown
    write_le_u32(page, offset, num_rows); offset += 4U;
    write_le_u32(page, offset, 0U); offset += 4U;  // autonumber
    page[offset] = 0x01U; offset += 1U;  // autonum_flag
    offset += 3U;  // unknown
    write_le_u32(page, offset, 0U); offset += 4U;  // ct_autonum
    offset += 8U;  // unknown
    page[offset] = table_type; offset += 1U;
    write_le_u16(page, offset, num_cols); offset += 2U;  // max_cols
    std::uint16_t num_var_cols = 0U;
    for (const auto& column : columns) {
        if (!column.fixed) {
            ++num_var_cols;
        }
    }
    write_le_u16(page, offset, num_var_cols); offset += 2U;
    write_le_u16(page, offset, num_cols); offset += 2U;  // num_cols
    write_le_u32(page, offset, 0U); offset += 4U;  // num_idx
    write_le_u32(page, offset, 0U); offset += 4U;  // num_real_idx = 0
    write_le_u32(page, offset, 0U); offset += 4U;  // used_pages
    write_le_u32(page, offset, 0U); offset += 4U;  // free_pages

    const std::vector<std::uint16_t> offset_f_table = compute_synthetic_offset_f_by_col_num_order(columns);
    for (std::size_t index = 0U; index < columns.size(); ++index) {
        const auto& column = columns[index];
        const auto col_num = column.column_number_override.value_or(static_cast<std::uint16_t>(index));
        page[offset] = column.type; offset += 1U;
        offset += 4U;  // unknown
        write_le_u16(page, offset, col_num); offset += 2U;  // col_num
        write_le_u16(page, offset, 0U); offset += 2U;  // offset_V
        write_le_u16(page, offset, col_num); offset += 2U;  // col_num (repeat)
        write_le_u16(page, offset, 0x409U); offset += 2U;  // misc
        write_le_u16(page, offset, 0U); offset += 2U;  // misc_ext
        page[offset] = column.fixed ? 0x01U : 0x00U; offset += 1U;  // bitmask
        page[offset] = 0U; offset += 1U;  // misc_flags
        offset += 4U;  // unknown
        write_le_u16(page, offset, offset_f_table[index]); offset += 2U;  // offset_F
        write_le_u16(page, offset, column.length); offset += 2U;  // col_len
    }
    for (const auto& column : columns) {
        write_le_u16(page, offset, static_cast<std::uint16_t>(column.name.size() * 2U)); offset += 2U;
        for (const char character : column.name) {
            page[offset] = static_cast<std::uint8_t>(character);
            page[offset + 1U] = 0U;
            offset += 2U;
        }
    }
    return page;
}

// #5539: synthetic MSysObjects data-row builder, reproducing the general
// Jet3/Jet4 row-decoding byte layout (null bitmask, reverse-order
// variable-length offset table, eod sentinel) independently cross-checked
// against real Jet3/Jet4 .mdb fixtures during this slice's development --
// see docs/72's worked byte-level walkthrough for the derivation. Unlike
// the TDEF builders above, `columns` here must already be listed in
// col_num (declaration) order: index i describes column i. `values[i]` is
// column i's byte content, or nullopt for a null column -- a present but
// empty variable-length value is an empty (not absent) vector, matching
// the null mask's own not-null-but-zero-length semantics this slice's
// row decoder relies on (see decode_row()'s own comment,
// access_msysobjects.cpp).
std::vector<std::uint8_t> make_synthetic_msysobjects_row_bytes(
    const std::vector<SyntheticAccessColumn>& columns,
    const std::vector<std::optional<std::vector<std::uint8_t>>>& values,
    bool is_jet3) {
    const std::size_t num_cols_field_width = is_jet3 ? 1U : 2U;
    const std::size_t entry_width = is_jet3 ? 1U : 2U;

    std::vector<std::uint8_t> fixed_bytes;
    for (std::size_t index = 0U; index < columns.size(); ++index) {
        if (!columns[index].fixed) {
            continue;
        }
        std::vector<std::uint8_t> slot(columns[index].length, 0U);
        if (values[index].has_value()) {
            const auto& value = *values[index];
            for (std::size_t byte_index = 0U; byte_index < slot.size() && byte_index < value.size(); ++byte_index) {
                slot[byte_index] = value[byte_index];
            }
        }
        fixed_bytes.insert(fixed_bytes.end(), slot.begin(), slot.end());
    }

    std::vector<std::uint8_t> var_bytes;
    std::vector<std::uint32_t> var_starts;
    for (std::size_t index = 0U; index < columns.size(); ++index) {
        if (columns[index].fixed) {
            continue;
        }
        var_starts.push_back(static_cast<std::uint32_t>(
            num_cols_field_width + fixed_bytes.size() + var_bytes.size()));
        if (values[index].has_value()) {
            const auto& value = *values[index];
            var_bytes.insert(var_bytes.end(), value.begin(), value.end());
        }
    }
    const auto eod = static_cast<std::uint32_t>(num_cols_field_width + fixed_bytes.size() + var_bytes.size());
    const std::size_t var_len = var_starts.size();

    const std::size_t null_mask_size = (columns.size() + 7U) / 8U;
    std::vector<std::uint8_t> null_mask(null_mask_size, 0U);
    for (std::size_t index = 0U; index < columns.size(); ++index) {
        if (values[index].has_value()) {
            null_mask[index / 8U] = static_cast<std::uint8_t>(null_mask[index / 8U] | (1U << (index % 8U)));
        }
    }

    auto push_entry = [&](std::vector<std::uint8_t>& out, std::uint32_t value) {
        if (is_jet3) {
            out.push_back(static_cast<std::uint8_t>(value));
        } else {
            out.push_back(static_cast<std::uint8_t>(value & 0xFFU));
            out.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
        }
    };

    std::vector<std::uint8_t> row;
    push_entry(row, static_cast<std::uint32_t>(columns.size()));
    row.insert(row.end(), fixed_bytes.begin(), fixed_bytes.end());
    row.insert(row.end(), var_bytes.begin(), var_bytes.end());
    push_entry(row, eod);
    for (std::size_t slot = var_len; slot-- > 0U;) {
        push_entry(row, var_starts[slot]);
    }
    push_entry(row, static_cast<std::uint32_t>(var_len));
    row.insert(row.end(), null_mask.begin(), null_mask.end());
    (void)entry_width;
    return row;
}

struct SyntheticDataRowSlot {
    std::vector<std::uint8_t> bytes;
    bool deleted = false;
    bool lookup_overflow = false;
};

// #5539: synthetic Jet3/Jet4 data-page builder, laying out its row-offset
// table and row content the way scan_access_msysobjects_catalog() expects
// (rows placed from the end of the page backward; a deleted or lookup-
// overflow row's own slot flag set rather than its content matching).
std::vector<std::uint8_t> make_synthetic_data_page(
    std::size_t page_size,
    std::uint32_t tdef_pg,
    bool is_jet3,
    const std::vector<SyntheticDataRowSlot>& rows) {
    std::vector<std::uint8_t> page(page_size, 0U);
    page[0] = 0x01U;  // data page
    write_le_u32(page, 4U, tdef_pg);
    const std::size_t header_size = is_jet3 ? 10U : 14U;
    const std::size_t num_rows_offset = is_jet3 ? 8U : 12U;
    write_le_u16(page, num_rows_offset, static_cast<std::uint16_t>(rows.size()));

    std::size_t cursor = page_size;
    std::vector<std::uint16_t> offsets(rows.size());
    for (std::size_t index = 0U; index < rows.size(); ++index) {
        const std::size_t length =
            rows[index].lookup_overflow ? 4U : (rows[index].deleted ? 0U : rows[index].bytes.size());
        const std::size_t start = cursor - length;
        if (!rows[index].deleted && !rows[index].lookup_overflow) {
            std::copy(rows[index].bytes.begin(), rows[index].bytes.end(), page.begin() + static_cast<std::ptrdiff_t>(start));
        }
        offsets[index] = static_cast<std::uint16_t>(start);
        cursor = start;
    }
    for (std::size_t index = 0U; index < rows.size(); ++index) {
        std::uint16_t flags = 0U;
        if (rows[index].deleted) {
            flags = 0x8000U;
        } else if (rows[index].lookup_overflow) {
            flags = 0x4000U;
        }
        write_le_u16(page, header_size + index * 2U, static_cast<std::uint16_t>(offsets[index] | flags));
    }
    return page;
}

std::vector<std::uint8_t> ucs2le_bytes(const std::string& ascii_text) {
    std::vector<std::uint8_t> bytes;
    bytes.reserve(ascii_text.size() * 2U);
    for (const char character : ascii_text) {
        bytes.push_back(static_cast<std::uint8_t>(character));
        bytes.push_back(0U);
    }
    return bytes;
}

std::vector<std::uint8_t> le_bytes32(std::uint32_t value) {
    return {
        static_cast<std::uint8_t>(value & 0xFFU),
        static_cast<std::uint8_t>((value >> 8U) & 0xFFU),
        static_cast<std::uint8_t>((value >> 16U) & 0xFFU),
        static_cast<std::uint8_t>((value >> 24U) & 0xFFU)};
}

std::vector<std::uint8_t> le_bytes16(std::uint16_t value) {
    return {static_cast<std::uint8_t>(value & 0xFFU), static_cast<std::uint8_t>((value >> 8U) & 0xFFU)};
}

// Writes a minimal but well-formed synthetic container file: a page-0
// container header for the given generation, a blank page 1, MSysObjects's
// own TDEF at page 2, and every page in `trailing_pages` starting at page
// 3 -- the same container shape
// test_scan_access_container_schema_discovers_tables_and_skips_continuations
// already established, reused here for #5539's row-decode tests.
std::filesystem::path write_synthetic_msysobjects_container(
    const std::filesystem::path& temp_dir,
    const std::string& filename,
    bool is_jet3,
    const std::vector<SyntheticAccessColumn>& msysobjects_columns,
    const std::vector<std::vector<std::uint8_t>>& trailing_pages,
    // #5543 review (Codex, P1 mitigation): scan_access_msysobjects_catalog()
    // now fails closed if it finds more non-deleted row slots than
    // MSysObjects's own TDEF declares via row_count (a partial guard
    // against stale/freed pages -- see that function's own comment).
    // Callers must pass the real number of non-deleted rows their
    // synthetic data pages contain for that check to pass.
    std::uint32_t msysobjects_row_count = 0U) {
    const std::size_t page_size = is_jet3 ? 2048U : 4096U;
    std::vector<std::uint8_t> file_bytes(page_size * (3U + trailing_pages.size()), 0U);

    file_bytes[0] = 0x00U;
    file_bytes[1] = 0x01U;
    file_bytes[2] = 0x00U;
    file_bytes[3] = 0x00U;
    const std::string signature = is_jet3 ? "Standard Jet DB" : "Standard ACE DB";
    std::copy(signature.begin(), signature.end(), file_bytes.begin() + 4);
    file_bytes[0x14] = is_jet3 ? 0x00U : 0x01U;

    const auto tdef_page = is_jet3
        ? make_synthetic_jet3_tdef_page(msysobjects_columns, 0x53U, msysobjects_row_count)
        : make_synthetic_jet4_tdef_page(msysobjects_columns, 0x53U, msysobjects_row_count);
    std::copy(tdef_page.begin(), tdef_page.end(), file_bytes.begin() + static_cast<std::ptrdiff_t>(page_size * 2U));

    for (std::size_t index = 0U; index < trailing_pages.size(); ++index) {
        std::copy(
            trailing_pages[index].begin(),
            trailing_pages[index].end(),
            file_bytes.begin() + static_cast<std::ptrdiff_t>(page_size * (3U + index)));
    }

    const std::filesystem::path container_path = temp_dir / filename;
    expect(write_binary_file(container_path, file_bytes), "synthetic MSysObjects container fixture should be writable");
    return container_path;
}

void test_parse_access_table_definition_page_decodes_jet3_columns() {
    const std::vector<SyntheticAccessColumn> columns{
        {.name = "Id", .type = 0x04U, .length = 4U, .fixed = true},
        {.name = "Name", .type = 0x0AU, .length = 50U, .fixed = false},
        {.name = "Flags", .type = 0x04U, .length = 4U, .fixed = true},
    };
    const auto page = make_synthetic_jet3_tdef_page(columns, 0x53U, 21U);

    const auto result = copperfin::vfp::parse_access_table_definition_page(
        page, copperfin::vfp::AccessContainerGeneration::jet3, 2U);
    expect(result.ok, "parse_access_table_definition_page should decode a well-formed Jet3 TDEF page: " + result.error);
    expect(result.page_number == 2U, "the decoded table should record its own page number");
    expect(result.is_system_table, "table_type 0x53 should classify as a system table");
    expect(result.row_count == 21U, "row_count should come from the TDEF block's num_rows field");
    expect(result.columns.size() == 3U, "every synthesized column should be decoded");
    if (result.columns.size() == 3U) {
        expect(result.columns[0].name == "Id" &&
                   result.columns[0].type == copperfin::vfp::AccessColumnType::long_integer &&
                   result.columns[0].length == 4U && result.columns[0].fixed_length,
               "the first column should decode as a fixed-length LONGINT named Id");
        expect(result.columns[1].name == "Name" &&
                   result.columns[1].type == copperfin::vfp::AccessColumnType::text &&
                   result.columns[1].length == 50U && !result.columns[1].fixed_length,
               "the second column should decode as a variable-length TEXT(50) named Name");
        expect(result.columns[2].name == "Flags" &&
                   result.columns[2].type == copperfin::vfp::AccessColumnType::long_integer,
               "the third column should decode as a LONGINT named Flags");
    }
}

void test_parse_access_table_definition_page_decodes_jet4_columns() {
    const std::vector<SyntheticAccessColumn> columns{
        {.name = "Id", .type = 0x04U, .length = 4U, .fixed = true},
        {.name = "Name", .type = 0x0AU, .length = 100U, .fixed = false},
    };
    const auto page = make_synthetic_jet4_tdef_page(columns, 0x4EU, 7U);

    const auto result = copperfin::vfp::parse_access_table_definition_page(
        page, copperfin::vfp::AccessContainerGeneration::jet4, 17U);
    expect(result.ok, "parse_access_table_definition_page should decode a well-formed Jet4 TDEF page: " + result.error);
    expect(!result.is_system_table, "table_type 0x4E should classify as a user table, not a system table");
    expect(result.row_count == 7U, "row_count should come from the TDEF block's num_rows field");
    expect(result.columns.size() == 2U, "every synthesized column should be decoded");
    if (result.columns.size() == 2U) {
        expect(result.columns[0].name == "Id" && result.columns[0].type == copperfin::vfp::AccessColumnType::long_integer,
               "the first column should decode as LONGINT named Id, with UCS-2 column names round-tripping to plain ASCII");
        expect(result.columns[1].name == "Name" && result.columns[1].length == 100U,
               "the second column should decode with its Jet4 byte-count length preserved");
    }
}

void test_parse_access_table_definition_page_rejects_wrong_page_size() {
    std::vector<std::uint8_t> undersized_page(100U, 0U);
    const auto result = copperfin::vfp::parse_access_table_definition_page(
        undersized_page, copperfin::vfp::AccessContainerGeneration::jet3, 2U);
    expect(!result.ok, "parse_access_table_definition_page should reject a page that isn't exactly the generation's page size");
}

void test_parse_access_table_definition_page_rejects_non_tdef_page() {
    std::vector<std::uint8_t> data_page(2048U, 0U);
    data_page[0] = 0x01U;  // data page, not a TDEF page
    const auto result = copperfin::vfp::parse_access_table_definition_page(
        data_page, copperfin::vfp::AccessContainerGeneration::jet3, 5U);
    expect(!result.ok, "parse_access_table_definition_page should reject a page whose leading byte isn't 0x02");
}

void test_parse_access_table_definition_page_rejects_multi_page_tdef() {
    auto page = make_synthetic_jet3_tdef_page({{.name = "Id", .type = 0x04U, .length = 4U, .fixed = true}}, 0x4EU, 0U);
    write_le_u32(page, 4U, 9U);  // next_pg != 0: a spanning TDEF this slice does not support
    const auto result = copperfin::vfp::parse_access_table_definition_page(
        page, copperfin::vfp::AccessContainerGeneration::jet3, 2U);
    expect(!result.ok, "parse_access_table_definition_page should fail closed on a multi-page TDEF rather than parse only the first page");
}

void test_parse_access_table_definition_page_rejects_structure_out_of_bounds() {
    auto page = make_synthetic_jet3_tdef_page({{.name = "Id", .type = 0x04U, .length = 4U, .fixed = true}}, 0x4EU, 0U);
    // Declare far more columns than the page actually has room for (the
    // column-name-length offset in the TDEF block), modeling a crafted or
    // corrupt page rather than trusting it to read past its own end.
    write_le_u16(page, 8U + 4U + 4U + 4U + 1U + 2U + 2U, 5000U);
    const auto result = copperfin::vfp::parse_access_table_definition_page(
        page, copperfin::vfp::AccessContainerGeneration::jet3, 2U);
    expect(!result.ok, "parse_access_table_definition_page should fail closed rather than read past the page for an inflated column count");
}

void test_scan_access_container_schema_discovers_tables_and_skips_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_schema_scan_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);
    const fs::path container_path = temp_dir / "container.mdb";

    constexpr std::size_t page_size = 2048U;
    std::vector<std::uint8_t> file_bytes(page_size * 5U, 0U);

    // Page 0: database definition page, carrying the container-level
    // signature/generation bytes this file already relies on
    // (parse_access_container_header_from_file()).
    file_bytes[0] = 0x00U;
    file_bytes[1] = 0x01U;
    file_bytes[2] = 0x00U;
    file_bytes[3] = 0x00U;
    const std::string signature = "Standard Jet DB";
    std::copy(signature.begin(), signature.end(), file_bytes.begin() + 4);
    file_bytes[0x14] = 0x00U;  // Jet3

    // Page 2: a genuine single-page TDEF (the discoverable table).
    const auto table_page = make_synthetic_jet3_tdef_page(
        {{.name = "Id", .type = 0x04U, .length = 4U, .fixed = true}}, 0x4EU, 3U);
    std::copy(table_page.begin(), table_page.end(), file_bytes.begin() + static_cast<std::ptrdiff_t>(page_size * 2U));

    // Page 3: a TDEF-typed page (leading byte 0x02) that is a continuation
    // of an earlier chain -- page 4 below points its next_pg at this page,
    // so it must not be counted as its own independent table.
    file_bytes[page_size * 3U] = 0x02U;

    // Page 4: a TDEF whose next_pg points at page 3 -- a multi-page TDEF
    // this slice deliberately fails closed on (see
    // test_parse_access_table_definition_page_rejects_multi_page_tdef),
    // reported in `skipped` rather than silently dropped or crashing the
    // whole scan.
    auto spanning_page = make_synthetic_jet3_tdef_page(
        {{.name = "Wide", .type = 0x0AU, .length = 50U, .fixed = false}}, 0x4EU, 0U);
    write_le_u32(spanning_page, 4U, 3U);
    std::copy(spanning_page.begin(), spanning_page.end(), file_bytes.begin() + static_cast<std::ptrdiff_t>(page_size * 4U));

    expect(write_binary_file(container_path, file_bytes), "the synthetic multi-page container fixture should be writable");

    const auto result = copperfin::vfp::scan_access_container_schema(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_container_schema should succeed for a well-formed synthetic container: " + result.error);
    expect(result.tables.size() == 1U, "only the genuine single-page TDEF at page 2 should be reported as a discovered table");
    if (result.tables.size() == 1U) {
        expect(result.tables[0].page_number == 2U, "the discovered table should be the one at page 2");
    }
    expect(result.skipped.size() == 1U, "the multi-page TDEF chain-start at page 4 should be reported as skipped, not silently dropped");
    if (result.skipped.size() == 1U) {
        expect(result.skipped[0].page_number == 4U, "the skipped entry should identify the chain-start page, not the continuation page");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_container_schema_rejects_non_access_file() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_schema_scan_rejection_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);
    const fs::path not_a_container = temp_dir / "not-access.bin";
    expect(write_binary_file(not_a_container, std::vector<std::uint8_t>(100U, 0xAAU)),
           "the non-Access fixture should be writable");

    const auto result = copperfin::vfp::scan_access_container_schema(
        copperfin::platform::path_to_utf8_string(not_a_container));
    expect(!result.ok, "scan_access_container_schema should reject a file with no recognizable Access container signature");

    fs::remove_all(temp_dir, ignored);
}

void test_parse_access_table_definition_page_rejects_oversized_index_count() {
    // #5540 review (Copilot): num_real_idx is untrusted page data multiplied
    // by a fixed per-entry stride (8 bytes for Jet3) to compute how far to
    // skip past the TDEF's index-info block. The original
    // count-then-multiply-then-bounds-check pattern could overflow for an
    // adversarial count, wrapping around to a small value and silently
    // bypassing the bounds check it was meant to enforce. A page claiming
    // an index count that could not possibly fit (0xFFFFFFFF entries in a
    // 2048-byte page) must fail closed rather than let that wraparound
    // misdirect the cursor into reading unrelated bytes as if they were
    // column descriptors.
    auto page = make_synthetic_jet3_tdef_page({{.name = "Id", .type = 0x04U, .length = 4U, .fixed = true}}, 0x4EU, 0U);
    // num_real_idx sits at byte 31 in the Jet3 layout (8-byte TDEF header +
    // tdef_len(4) + num_rows(4) + autonumber(4) + table_type(1) +
    // max_cols(2) + num_var_cols(2) + num_cols(2) + num_idx(4) = 31).
    write_le_u32(page, 31U, 0xFFFFFFFFU);
    const auto result = copperfin::vfp::parse_access_table_definition_page(
        page, copperfin::vfp::AccessContainerGeneration::jet3, 2U);
    expect(!result.ok, "parse_access_table_definition_page should fail closed on an index count that cannot possibly fit in the page");
}

void test_scan_access_container_schema_rejects_truncated_trailing_page() {
    // #5540 review (Copilot): a file size that isn't an exact multiple of
    // the generation's page size used to be silently truncated via integer
    // division, leaving a real (but incomplete) trailing page unexamined
    // without any indication anything was skipped. A truncated/corrupt
    // container should fail closed instead.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_schema_scan_truncated_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);
    const fs::path container_path = temp_dir / "truncated.mdb";

    constexpr std::size_t page_size = 2048U;
    std::vector<std::uint8_t> file_bytes(page_size * 2U + 10U, 0U);  // 10 stray trailing bytes
    file_bytes[0] = 0x00U;
    file_bytes[1] = 0x01U;
    file_bytes[2] = 0x00U;
    file_bytes[3] = 0x00U;
    const std::string signature = "Standard Jet DB";
    std::copy(signature.begin(), signature.end(), file_bytes.begin() + 4);
    file_bytes[0x14] = 0x00U;  // Jet3

    expect(write_binary_file(container_path, file_bytes), "the truncated container fixture should be writable");

    const auto result = copperfin::vfp::scan_access_container_schema(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(!result.ok, "scan_access_container_schema should reject a file size that isn't an exact multiple of the page size");

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_container_schema_excludes_every_page_in_a_multi_hop_chain() {
    // #5540 review (Copilot): continuation-page exclusion originally only
    // followed the first next_pg hop. A TDEF chain longer than two pages
    // (page A -> B -> C) would leave C unmarked -- and since C's own
    // next_pg is 0 (the natural end of the chain), C could be misreported
    // as its own independent, successfully-parsed single-page table,
    // rather than the continuation data it actually is.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_schema_scan_multihop_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);
    const fs::path container_path = temp_dir / "multihop.mdb";

    constexpr std::size_t page_size = 2048U;
    std::vector<std::uint8_t> file_bytes(page_size * 5U, 0U);
    file_bytes[0] = 0x00U;
    file_bytes[1] = 0x01U;
    file_bytes[2] = 0x00U;
    file_bytes[3] = 0x00U;
    const std::string signature = "Standard Jet DB";
    std::copy(signature.begin(), signature.end(), file_bytes.begin() + 4);
    file_bytes[0x14] = 0x00U;  // Jet3

    // Page 2 (chain start): next_pg = 3.
    auto page_a = make_synthetic_jet3_tdef_page({{.name = "Id", .type = 0x04U, .length = 4U, .fixed = true}}, 0x4EU, 0U);
    write_le_u32(page_a, 4U, 3U);
    std::copy(page_a.begin(), page_a.end(), file_bytes.begin() + static_cast<std::ptrdiff_t>(page_size * 2U));

    // Page 3 (middle of the chain): next_pg = 4.
    auto page_b = make_synthetic_jet3_tdef_page({{.name = "Mid", .type = 0x04U, .length = 4U, .fixed = true}}, 0x4EU, 0U);
    write_le_u32(page_b, 4U, 4U);
    std::copy(page_b.begin(), page_b.end(), file_bytes.begin() + static_cast<std::ptrdiff_t>(page_size * 3U));

    // Page 4 (chain end): next_pg = 0, so it looks exactly like a
    // genuine, independently-parseable single-page TDEF if the multi-hop
    // walk fails to reach it.
    auto page_c = make_synthetic_jet3_tdef_page({{.name = "End", .type = 0x04U, .length = 4U, .fixed = true}}, 0x4EU, 0U);
    std::copy(page_c.begin(), page_c.end(), file_bytes.begin() + static_cast<std::ptrdiff_t>(page_size * 4U));

    expect(write_binary_file(container_path, file_bytes), "the multi-hop chain container fixture should be writable");

    const auto result = copperfin::vfp::scan_access_container_schema(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_container_schema should succeed for a well-formed synthetic container: " + result.error);
    expect(result.tables.empty(),
           "no page in a 3-page TDEF chain should be reported as an independent table, including the chain's own end page");
    expect(result.skipped.size() == 1U,
           "only the chain's own start page should be reported (as an unsupported multi-page TDEF), not its continuation pages");
    if (result.skipped.size() == 1U) {
        expect(result.skipped[0].page_number == 2U, "the skipped entry should identify the chain-start page");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_parse_access_table_definition_page_sanitizes_invalid_jet3_utf8() {
    // #5540 review (Codex): Jet3 column names are stored in the database's
    // legacy single-byte code page, not UTF-8. This codebase doesn't yet
    // read that code page (it lives on the RC4-"encrypted" Database
    // Definition page, decryption of which is out of scope for this
    // slice), so a real non-ASCII name can't be correctly transcoded --
    // but it must still never come back as invalid UTF-8, which could
    // corrupt whatever downstream code assumes every Copperfin string is
    // valid UTF-8. 0xC9 alone (a lone continuation-less high byte -- e.g.
    // 'É' in Windows-1252) is not valid UTF-8 on its own.
    std::vector<SyntheticAccessColumn> columns{
        {.name = "Ab", .type = 0x04U, .length = 4U, .fixed = true},
    };
    auto page = make_synthetic_jet3_tdef_page(columns, 0x4EU, 0U);
    // Patch the first column's name bytes directly: name_len=1, followed
    // by the single invalid byte 0xC9, replacing the synthesized "Ab".
    // The column-descriptor array is 1 column * 18 bytes after the fixed
    // 31-byte block header (8 + 4+4+4+1+2+2+2+4+4+4+4 = 43), so the name
    // area starts at 43 + 18 = 61.
    constexpr std::size_t name_area_offset = 61U;
    page[name_area_offset] = 1U;
    page[name_area_offset + 1U] = 0xC9U;

    const auto result = copperfin::vfp::parse_access_table_definition_page(
        page, copperfin::vfp::AccessContainerGeneration::jet3, 2U);
    expect(result.ok, "parse_access_table_definition_page should still decode a page with an invalid-UTF-8 name byte: " + result.error);
    if (result.ok && result.columns.size() == 1U) {
        expect(result.columns[0].name == "\xEF\xBF\xBD",
               "an invalid Jet3 name byte should become U+FFFD rather than being passed through as invalid UTF-8");
    }
}

void test_parse_access_table_definition_page_rejects_unrecognized_table_type() {
    // #5540 review (Codex): table_type is only documented to be 0x4E
    // (user table) or 0x53 (system table). The original version silently
    // mapped any other value to is_system_table == false, letting a
    // corrupt or falsely-detected TDEF page report ok=true with
    // fabricated schema data instead of failing closed.
    auto page = make_synthetic_jet3_tdef_page(
        {{.name = "Id", .type = 0x04U, .length = 4U, .fixed = true}}, 0x00U, 0U);
    const auto result = copperfin::vfp::parse_access_table_definition_page(
        page, copperfin::vfp::AccessContainerGeneration::jet3, 2U);
    expect(!result.ok, "parse_access_table_definition_page should reject an unrecognized table_type byte rather than default to a user table");
}

// #5539: a minimal MSysObjects-shaped schema for row-decode tests --
// Id/ParentId/Type fixed, Name/Extra variable (Extra always left null in
// most tests, exercising the "declared variable column with no data"
// path every real MSysObjects row also has for its many usually-empty
// OLE/memo columns).
std::vector<SyntheticAccessColumn> make_msysobjects_test_columns() {
    return {
        {.name = "Id", .type = 0x04U, .length = 4U, .fixed = true},
        {.name = "ParentId", .type = 0x04U, .length = 4U, .fixed = true},
        {.name = "Name", .type = 0x0AU, .length = 255U, .fixed = false},
        {.name = "Type", .type = 0x03U, .length = 2U, .fixed = true},
        {.name = "Extra", .type = 0x0AU, .length = 255U, .fixed = false},
    };
}

void test_scan_access_msysobjects_catalog_decodes_jet3_row() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_msysobjects_jet3_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto columns = make_msysobjects_test_columns();
    const std::vector<std::optional<std::vector<std::uint8_t>>> values{
        le_bytes32(2U),
        le_bytes32(251658241U),
        std::vector<std::uint8_t>{'M', 'S', 'y', 's', 'O', 'b', 'j', 'e', 'c', 't', 's'},
        le_bytes16(1U),
        std::nullopt,
    };
    const auto row = make_synthetic_msysobjects_row_bytes(columns, values, true);
    const auto data_page = make_synthetic_data_page(2048U, 2U, true, {{.bytes = row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "jet3.mdb", true, columns, {data_page}, 1U);

    const auto result = copperfin::vfp::scan_access_msysobjects_catalog(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_msysobjects_catalog should succeed for a well-formed synthetic Jet3 row: " + result.error);
    expect(result.skipped.empty(), "a well-formed row should not be reported as skipped");
    expect(result.entries.size() == 1U, "exactly one MSysObjects row should be decoded");
    if (result.entries.size() == 1U) {
        const auto& entry = result.entries[0];
        expect(entry.id == 2U, "decoded Id should match the synthetic row's fixed-column value");
        expect(entry.parent_id == 251658241U, "decoded ParentId should match the synthetic row's fixed-column value");
        expect(entry.type == 1, "decoded Type should match the synthetic row's fixed-column value");
        expect(entry.name == "MSysObjects", "decoded Name should match the synthetic row's variable-column value");
        expect(entry.candidate_page_number == 2U, "candidate_page_number should be Id masked to its low 24 bits");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_msysobjects_catalog_decodes_jet4_row() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_msysobjects_jet4_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto columns = make_msysobjects_test_columns();
    const std::vector<std::optional<std::vector<std::uint8_t>>> values{
        le_bytes32(26U),
        le_bytes32(251658241U),
        ucs2le_bytes("Customers"),
        le_bytes16(1U),
        std::nullopt,
    };
    const auto row = make_synthetic_msysobjects_row_bytes(columns, values, false);
    const auto data_page = make_synthetic_data_page(4096U, 2U, false, {{.bytes = row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "jet4.mdb", false, columns, {data_page}, 1U);

    const auto result = copperfin::vfp::scan_access_msysobjects_catalog(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_msysobjects_catalog should succeed for a well-formed synthetic Jet4 row: " + result.error);
    expect(result.entries.size() == 1U, "exactly one MSysObjects row should be decoded");
    if (result.entries.size() == 1U) {
        const auto& entry = result.entries[0];
        expect(entry.id == 26U, "decoded Id should match the synthetic row's fixed-column value");
        expect(entry.name == "Customers", "decoded Jet4 plain UCS-2LE Name should be transcoded to UTF-8 correctly");
        expect(entry.candidate_page_number == 26U, "candidate_page_number should be Id masked to its low 24 bits");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_msysobjects_catalog_handles_alphabetized_column_storage_order() {
    // #5539: a real Jet4 fixture found during this slice's development
    // stores its MSysObjects TDEF column descriptors in PHYSICAL storage
    // order alphabetized by name, not in col_num (logical declaration)
    // order -- e.g. "Connect" physically first even though its col_num
    // is 9. An earlier version of this decoder assumed physical vector
    // position could be used directly as both the null-mask bit index
    // and the well-known Id/ParentId/Name/Type column lookup, which
    // silently decoded garbage against that real fixture. This
    // reproduces that same physical/logical mismatch against a
    // synthetic fixture as a regression test.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_msysobjects_alphabetized_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    // Declared (TDEF descriptor) order: Extra, Name, ParentId, Id, Type --
    // deliberately not matching col_num order -- with column_number_override
    // restoring the real logical order (Id=0, ParentId=1, Name=2, Type=3,
    // Extra=4).
    const std::vector<SyntheticAccessColumn> columns{
        {.name = "Extra", .type = 0x0AU, .length = 255U, .fixed = false, .column_number_override = 4U},
        {.name = "Name", .type = 0x0AU, .length = 255U, .fixed = false, .column_number_override = 2U},
        {.name = "ParentId", .type = 0x04U, .length = 4U, .fixed = true, .column_number_override = 1U},
        {.name = "Id", .type = 0x04U, .length = 4U, .fixed = true, .column_number_override = 0U},
        {.name = "Type", .type = 0x03U, .length = 2U, .fixed = true, .column_number_override = 3U},
    };
    // Row-content values must still be supplied in col_num order (0..4),
    // matching make_synthetic_msysobjects_row_bytes()'s own contract --
    // the row bytes on disk are logically ordered regardless of how the
    // TDEF happens to physically store its column descriptors.
    const std::vector<SyntheticAccessColumn> columns_in_col_num_order = make_msysobjects_test_columns();
    const std::vector<std::optional<std::vector<std::uint8_t>>> values{
        le_bytes32(2U),
        le_bytes32(251658241U),
        std::vector<std::uint8_t>{'M', 'S', 'y', 's', 'O', 'b', 'j', 'e', 'c', 't', 's'},
        le_bytes16(1U),
        std::nullopt,
    };
    const auto row = make_synthetic_msysobjects_row_bytes(columns_in_col_num_order, values, true);
    const auto data_page = make_synthetic_data_page(2048U, 2U, true, {{.bytes = row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "alphabetized.mdb", true, columns, {data_page}, 1U);

    const auto result = copperfin::vfp::scan_access_msysobjects_catalog(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_msysobjects_catalog should succeed even when the TDEF's physical column order "
                       "does not match col_num order: " + result.error);
    expect(result.entries.size() == 1U, "exactly one MSysObjects row should be decoded");
    if (result.entries.size() == 1U) {
        const auto& entry = result.entries[0];
        expect(entry.id == 2U, "decoded Id should be correct despite the TDEF's alphabetized physical column order");
        expect(entry.parent_id == 251658241U, "decoded ParentId should be correct despite the alphabetized physical column order");
        expect(entry.type == 1, "decoded Type should be correct despite the alphabetized physical column order");
        expect(entry.name == "MSysObjects", "decoded Name should be correct despite the alphabetized physical column order");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_msysobjects_catalog_skips_deleted_and_lookup_overflow_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_msysobjects_flags_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto columns = make_msysobjects_test_columns();
    const std::vector<std::optional<std::vector<std::uint8_t>>> values{
        le_bytes32(2U),
        le_bytes32(251658241U),
        std::vector<std::uint8_t>{'M', 'S', 'y', 's', 'O', 'b', 'j', 'e', 'c', 't', 's'},
        le_bytes16(1U),
        std::nullopt,
    };
    const auto row = make_synthetic_msysobjects_row_bytes(columns, values, true);
    const auto data_page = make_synthetic_data_page(
        2048U, 2U, true,
        {
            {.bytes = {}, .deleted = true},
            {.bytes = {}, .lookup_overflow = true},
            {.bytes = row},
        });

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "flags.mdb", true, columns, {data_page}, 2U);

    const auto result = copperfin::vfp::scan_access_msysobjects_catalog(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_msysobjects_catalog should succeed: " + result.error);
    expect(result.entries.size() == 1U, "only the one genuine row should be decoded; the deleted row should be silently excluded");
    expect(result.skipped.size() == 1U, "the lookup-overflow row should be reported as skipped, not silently dropped");
    if (result.skipped.size() == 1U) {
        expect(
            result.skipped[0].reason.find("lookup-overflow") != std::string::npos ||
                result.skipped[0].reason.find("does not follow") != std::string::npos,
            "the skipped entry's reason should describe the lookup-overflow pointer, got: " + result.skipped[0].reason);
    }

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_msysobjects_catalog_fails_closed_on_jet3_row_at_or_above_256_bytes() {
    // #5539: Jet3 offsets are 1-byte fields (max 256), requiring a jump
    // table this slice deliberately does not implement (see
    // access_msysobjects.h's own documented non-goal -- no real fixture
    // available during this slice's development ever produced a row
    // this large, so an unverified interpretation of the documented
    // jump-table algorithm was not shipped). This proves the >= 256-byte
    // path fails closed (reported in `skipped`) rather than silently
    // misreading offsets.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_msysobjects_jumptable_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto columns = make_msysobjects_test_columns();
    const std::vector<std::uint8_t> oversized_extra(280U, static_cast<std::uint8_t>('x'));
    const std::vector<std::optional<std::vector<std::uint8_t>>> values{
        le_bytes32(2U),
        le_bytes32(251658241U),
        std::vector<std::uint8_t>{'M', 'S', 'y', 's', 'O', 'b', 'j', 'e', 'c', 't', 's'},
        le_bytes16(1U),
        oversized_extra,
    };
    const auto row = make_synthetic_msysobjects_row_bytes(columns, values, true);
    expect(row.size() >= 256U, "the synthetic row fixture should genuinely be >= 256 bytes for this test to be meaningful");
    const auto data_page = make_synthetic_data_page(2048U, 2U, true, {{.bytes = row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "jumptable.mdb", true, columns, {data_page}, 1U);

    const auto result = copperfin::vfp::scan_access_msysobjects_catalog(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_msysobjects_catalog should still succeed overall: " + result.error);
    expect(result.entries.empty(), "a >= 256-byte Jet3 row should not be decoded");
    expect(result.skipped.size() == 1U, "a >= 256-byte Jet3 row should be reported as skipped, not silently dropped or misread");

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_msysobjects_catalog_fails_closed_on_jet4_compressed_unicode_name() {
    // #5539: every real Jet4 Name value observed during this slice's
    // development was plain (uncompressed) UCS-2LE. A value beginning
    // with the 0xFF 0xFE "compressed unicode" marker documented in
    // mdbtools' HACKING.md is deliberately not decoded (see
    // access_msysobjects.h's own documented non-goal) -- this proves
    // that path fails closed rather than mis-decoding the marker bytes
    // as if they were ordinary UCS-2 characters.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_msysobjects_compressed_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto columns = make_msysobjects_test_columns();
    const std::vector<std::optional<std::vector<std::uint8_t>>> values{
        le_bytes32(2U),
        le_bytes32(251658241U),
        std::vector<std::uint8_t>{0xFFU, 0xFEU, 'X', 0x00U},
        le_bytes16(1U),
        std::nullopt,
    };
    const auto row = make_synthetic_msysobjects_row_bytes(columns, values, false);
    const auto data_page = make_synthetic_data_page(4096U, 2U, false, {{.bytes = row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "compressed.mdb", false, columns, {data_page}, 1U);

    const auto result = copperfin::vfp::scan_access_msysobjects_catalog(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_msysobjects_catalog should still succeed overall: " + result.error);
    expect(result.entries.empty(), "a compressed-unicode Name value should not be decoded");
    expect(result.skipped.size() == 1U, "a compressed-unicode Name value should be reported as skipped, not silently dropped or misread");

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_container_schema_attaches_names_from_catalog() {
    // #5539's actual deliverable: scan_access_container_schema() giving
    // discovered tables real names instead of only page numbers, by
    // decoding MSysObjects's own rows and matching each Type == 1 row's
    // candidate TDEF page number against a table this scan already
    // discovered independently (see that function's own comment for why
    // an unmatched candidate page is left unattached rather than
    // fabricating a table entry from the catalog row alone).
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_schema_scan_names_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto msysobjects_columns = make_msysobjects_test_columns();
    // A "real" user table's own TDEF, discoverable at page 3.
    const auto user_table_page = make_synthetic_jet3_tdef_page(
        {{.name = "WidgetId", .type = 0x04U, .length = 4U, .fixed = true}}, 0x4EU, 0U);

    // Two MSysObjects catalog rows: MSysObjects naming itself (page 2),
    // and a catalog row naming the user table at page 3.
    const std::vector<std::optional<std::vector<std::uint8_t>>> self_values{
        le_bytes32(2U), le_bytes32(251658241U),
        std::vector<std::uint8_t>{'M', 'S', 'y', 's', 'O', 'b', 'j', 'e', 'c', 't', 's'},
        le_bytes16(1U), std::nullopt,
    };
    const std::vector<std::optional<std::vector<std::uint8_t>>> widgets_values{
        le_bytes32(3U), le_bytes32(251658241U),
        std::vector<std::uint8_t>{'W', 'i', 'd', 'g', 'e', 't', 's'},
        le_bytes16(1U), std::nullopt,
    };
    const auto self_row = make_synthetic_msysobjects_row_bytes(msysobjects_columns, self_values, true);
    const auto widgets_row = make_synthetic_msysobjects_row_bytes(msysobjects_columns, widgets_values, true);
    const auto data_page = make_synthetic_data_page(2048U, 2U, true, {{.bytes = self_row}, {.bytes = widgets_row}});

    std::vector<std::uint8_t> file_bytes(2048U * 5U, 0U);
    file_bytes[0] = 0x00U;
    file_bytes[1] = 0x01U;
    file_bytes[2] = 0x00U;
    file_bytes[3] = 0x00U;
    const std::string signature = "Standard Jet DB";
    std::copy(signature.begin(), signature.end(), file_bytes.begin() + 4);
    file_bytes[0x14] = 0x00U;
    const auto tdef_page = make_synthetic_jet3_tdef_page(msysobjects_columns, 0x53U, 2U);
    std::copy(tdef_page.begin(), tdef_page.end(), file_bytes.begin() + static_cast<std::ptrdiff_t>(2048U * 2U));
    std::copy(user_table_page.begin(), user_table_page.end(), file_bytes.begin() + static_cast<std::ptrdiff_t>(2048U * 3U));
    std::copy(data_page.begin(), data_page.end(), file_bytes.begin() + static_cast<std::ptrdiff_t>(2048U * 4U));

    const fs::path container_path = temp_dir / "named_schema.mdb";
    expect(write_binary_file(container_path, file_bytes), "the synthetic named-schema container fixture should be writable");

    const auto result = copperfin::vfp::scan_access_container_schema(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_container_schema should succeed: " + result.error);
    expect(result.tables.size() == 2U, "both the MSysObjects TDEF and the user table's TDEF should be discovered");
    bool found_msysobjects = false;
    bool found_widgets = false;
    for (const auto& table : result.tables) {
        if (table.page_number == 2U) {
            expect(table.name.has_value() && *table.name == "MSysObjects",
                   "the MSysObjects table itself should be named from its own catalog row");
            found_msysobjects = true;
        } else if (table.page_number == 3U) {
            expect(table.name.has_value() && *table.name == "Widgets",
                   "the user table at page 3 should be named from the matching catalog row");
            found_widgets = true;
        }
    }
    expect(found_msysobjects, "the discovered tables should include page 2 (MSysObjects)");
    expect(found_widgets, "the discovered tables should include page 3 (the user table)");

    fs::remove_all(temp_dir, ignored);
}

void test_vfp_locale_catalog_parity() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> keys{
        "Vfp.AccessContainer.Error.HeaderTooSmall",
        "Vfp.AccessContainer.Error.OpenFileFailed",
        "Vfp.AccessContainer.Error.ReadHeaderFailed",
        "Vfp.AccessContainer.Error.SignatureMismatch",
        "Vfp.AccessMSysObjects.Error.ColumnCountMismatch",
        "Vfp.AccessMSysObjects.Error.CompressedUnicodeUnsupported",
        "Vfp.AccessMSysObjects.Error.JumpTableUnsupported",
        "Vfp.AccessMSysObjects.Error.LookupOverflowUnsupported",
        "Vfp.AccessMSysObjects.Error.NotAnAccessContainer",
        "Vfp.AccessMSysObjects.Error.OpenFileFailed",
        "Vfp.AccessMSysObjects.Error.ReadPageFailed",
        "Vfp.AccessMSysObjects.Error.RowCountExceedsDeclared",
        "Vfp.AccessMSysObjects.Error.RowStructureInvalid",
        "Vfp.AccessMSysObjects.Error.RowTooShort",
        "Vfp.AccessMSysObjects.Error.UnexpectedSchema",
        "Vfp.AccessMSysObjects.Error.UnknownGeneration",
        "Vfp.AccessTableDefinition.Error.MultiPageTdefUnsupported",
        "Vfp.AccessTableDefinition.Error.NotATdefPage",
        "Vfp.AccessTableDefinition.Error.NotAnAccessContainer",
        "Vfp.AccessTableDefinition.Error.OpenFileFailed",
        "Vfp.AccessTableDefinition.Error.ReadPageFailed",
        "Vfp.AccessTableDefinition.Error.StructureOutOfBounds",
        "Vfp.AccessTableDefinition.Error.UnrecognizedTableType",
        "Vfp.AccessTableDefinition.Error.UnknownGeneration",
        "Vfp.AccessTableDefinition.Error.WrongPageSize",
        "Vfp.AssetInspector.Error.DbcHeaderParseFailed",
        "Vfp.AssetInspector.Error.DbcPathMissing",
        "Vfp.AssetInspector.Error.DbcReadFailed",
        "Vfp.AssetInspector.Error.PathMissing",
        "Vfp.AssetInspector.Error.ReadFailed",
        "Vfp.AssetInspector.Validation.DbcCatalogEmpty",
        "Vfp.AssetInspector.Validation.DbcCatalogParseFailed",
        "Vfp.AssetInspector.Validation.DbfDescriptorSpanMisaligned",
        "Vfp.AssetInspector.Validation.DbfDescriptorTerminatorMissing",
        "Vfp.AssetInspector.Validation.DbfDescriptorTerminatorNoRoom",
        "Vfp.AssetInspector.Validation.DbfFieldLayoutOverflow",
        "Vfp.AssetInspector.Validation.DbfFieldLayoutOverlap",
        "Vfp.AssetInspector.Validation.DbfFieldNameBlank",
        "Vfp.AssetInspector.Validation.DbfFieldNameDuplicate",
        "Vfp.AssetInspector.Validation.DbfFieldNameInvalid",
        "Vfp.AssetInspector.Validation.DbfFieldOffsetInvalid",
        "Vfp.AssetInspector.Validation.DbfHeaderLengthDescriptorMismatch",
        "Vfp.AssetInspector.Validation.DbfHeaderLengthExceedsFileSize",
        "Vfp.AssetInspector.Validation.DbfRecordLengthMismatch",
        "Vfp.AssetInspector.Validation.DbfRecordStorageLengthMismatch",
        "Vfp.AssetInspector.Validation.DbfRecordStorageTruncated",
        "Vfp.AssetInspector.Validation.IndexCompanionParseFailed",
        "Vfp.AssetInspector.Validation.IndexStructuralSidecarMissing",
        "Vfp.AssetInspector.Validation.MemoBlockSizeInvalid",
        "Vfp.AssetInspector.Validation.MemoPayloadTruncated",
        "Vfp.AssetInspector.Validation.MemoPointerOutOfRange",
        "Vfp.AssetInspector.Validation.MemoSidecarHeaderTruncated",
        "Vfp.AssetInspector.Validation.MemoSidecarMissing",
        "Vfp.AssetInspector.Validation.MemoSidecarShorterThanBlockSize",
        "Vfp.CdxHeader.Error.InvalidValues",
        "Vfp.CdxHeader.Error.OpenFileFailed",
        "Vfp.CdxHeader.Error.ReadProbeFailed",
        "Vfp.CdxHeader.Error.ShortProbe",
        "Vfp.DbfHeader.Error.InvalidValues",
        "Vfp.DbfHeader.Error.OpenFileFailed",
        "Vfp.DbfHeader.Error.ReadHeaderFailed",
        "Vfp.DbfHeader.Error.ShortHeader",
        "Vfp.DbfHeader.Version.DbaseIiiCompatible",
        "Vfp.DbfHeader.Version.DbaseIiiMemo",
        "Vfp.DbfHeader.Version.DbaseIvMemo",
        "Vfp.DbfHeader.Version.DbaseIvMemoSql",
        "Vfp.DbfHeader.Version.DbaseIvSqlTable",
        "Vfp.DbfHeader.Version.DbaseIvSystemFile",
        "Vfp.DbfHeader.Version.DbaseLevel7",
        "Vfp.DbfHeader.Version.DbaseLevel7Memo",
        "Vfp.DbfHeader.Version.DbaseLevel7MemoSql",
        "Vfp.DbfHeader.Version.DbaseLevel7SqlTable",
        "Vfp.DbfHeader.Version.FoxProMemo",
        "Vfp.DbfHeader.Version.Foxbase",
        "Vfp.DbfHeader.Version.Unknown",
        "Vfp.DbfHeader.Version.VisualFoxPro",
        "Vfp.DbfHeader.Version.VisualFoxProAutoincrement",
        "Vfp.DbfHeader.Version.VisualFoxProVarbinaryVarchar",
        "Vfp.IndexProbe.Error.ClipperNtxHeaderTooSmall",
        "Vfp.IndexProbe.Error.ClipperNtxInvalidValues",
        "Vfp.IndexProbe.Error.DbaseMdxInvalidValues",
        "Vfp.IndexProbe.Error.DbaseMdxProbeTooSmall",
        "Vfp.IndexProbe.Error.DbaseMdxTagMetadataMissing",
        "Vfp.IndexProbe.Error.DbaseNdxHeaderTooSmall",
        "Vfp.IndexProbe.Error.DbaseNdxInvalidValues",
        "Vfp.IndexProbe.Error.OpenFileFailed",
        "Vfp.IndexProbe.Error.PathExtensionUnknown",
        "Vfp.IndexProbe.Error.ReadHeaderFailed",
        "Vfp.IndexProbe.Error.UnknownExtension",
        "Vfp.IndexProbe.Error.UnsupportedType",
        "Vfp.IndexProbe.Error.VisualFoxProIdxHeaderTooSmall",
        "Vfp.IndexProbe.Error.VisualFoxProIdxInvalidValues"};

    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", keys) == 0U,
        "#2602: es-419 should define every remaining non-DBF-table Vfp localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", keys) == 0U,
        "#2602: pt-BR should define every remaining non-DBF-table Vfp localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", keys) == 0U,
        "#2602: qps-ploc should define every remaining non-DBF-table Vfp localization key");
}

void test_inspect_database_container_collects_casefolded_same_base_companions() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_dbc_casefold_assets_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "sample.dbc";
    const fs::path dcx_path = temp_dir / "SAMPLE.DCX";

    {
        auto bytes = make_vfp_header();
        std::ofstream output(dbc_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        const auto bytes = make_synthetic_cdx_family_bytes(false, true);
        std::ofstream output(dcx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto result = copperfin::vfp::inspect_asset(dbc_path.string());
    expect(result.ok, "inspect_asset should succeed for a DBC with a case-folded DCX companion");
    expect(result.indexes.size() == 1U, "inspect_asset should collect a case-folded same-base DCX companion");
    expect(
        !has_validation_issue(result, "index.structural_sidecar_missing", dbc_path.string()),
        "inspect_asset should not report a missing structural companion when an uppercase DCX exists");

    fs::remove_all(temp_dir, ignored);
}

void test_inspect_database_container_extracts_first_pass_catalog_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_dbc_catalog_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "catalog.dbc";
    const fs::path dcx_path = temp_dir / "catalog.dcx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 81U, .length = 4U, .decimal_count = 0U}
    };

    const std::vector<std::vector<std::string>> records{
        {"DATABASE", "Northwind", "", ""},
        {"TABLE", "Customers", "Northwind", ""},
        {"VIEW", "ActiveCustomers", "Northwind", ""},
        {"RELATION", "OrdersToCustomers", "Northwind", ""},
        {"CONNECTION", "RemoteSql", "Northwind", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(dbc_path.string(), fields, records);
    expect(create_result.ok, "DBC fixture creation should succeed for catalog metadata coverage");

    {
        const auto bytes = make_synthetic_cdx_family_bytes(false, true);
        std::ofstream output(dcx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto result = copperfin::vfp::inspect_asset(dbc_path.string());
    expect(result.ok, "inspect_asset should succeed for a synthetic DBC catalog fixture");
    expect(result.database_container_metadata_available, "inspect_asset should expose first-pass DBC catalog metadata");
    if (result.database_container_metadata_available) {
        expect(result.database_container_metadata.available, "DBC metadata container should be marked available");
        expect(result.database_container_metadata.total_objects == 5U, "DBC metadata should count all catalog rows");
        expect(result.database_container_metadata.database_objects == 1U, "DBC metadata should count DATABASE rows");
        expect(result.database_container_metadata.table_objects == 1U, "DBC metadata should count TABLE rows");
        expect(result.database_container_metadata.view_objects == 1U, "DBC metadata should count VIEW rows");
        expect(result.database_container_metadata.relation_objects == 1U, "DBC metadata should count RELATION rows");
        expect(result.database_container_metadata.connection_objects == 1U, "DBC metadata should count CONNECTION rows");
        expect(!result.database_container_metadata.objects_preview.empty(), "DBC metadata should include object previews");
        if (!result.database_container_metadata.objects_preview.empty()) {
            const auto& first = result.database_container_metadata.objects_preview.front();
            expect(first.object_type_hint == "database", "DBC preview should normalize object-type hints");
            expect(first.object_name_hint == "Northwind", "DBC preview should preserve object-name hints");
        }
    }

    expect(
        !has_validation_issue(result, "dbc.catalog_parse_failed", "catalog.dbc"),
        "DBC catalog fixtures should not report catalog-parse failures");

    fs::remove_all(temp_dir, ignored);
}

void test_inspect_asset_resolves_explicit_unicode_memo_sidecar() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string("copperfin_vfp_explicit_memo_caf\xC3\xA9_tests");
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "explicit_sidecar.scx";
    const fs::path sidecar_path = temp_dir /
        copperfin::platform::path_from_utf8_string("memo_caf\xC3\xA9.sct");
    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }
    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo";
    }

    const auto result = copperfin::vfp::inspect_asset(
        copperfin::platform::path_to_utf8_string(form_path),
        copperfin::platform::path_to_utf8_string(sidecar_path));
    expect(result.ok, "inspect_asset should accept an explicit Unicode memo sidecar path");
    expect(
        !has_validation_issue(result, "memo.sidecar_missing", "memo_caf\xC3\xA9.sct"),
        "inspect_asset should resolve an explicit Unicode memo sidecar path");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_json_resolves_unicode_catalog_table_path() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_unicode_table_export_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir /
        copperfin::platform::path_from_utf8_string("caf\xC3\xA9table.dbf");
    const std::string table_name = "caf\xC3\xA9table";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 81U, .length = 4U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> dbc_records{
        {"DATABASE", "UnicodeRuntime", "", ""},
        {"TABLE", table_name, "UnicodeRuntime", ""}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields, dbc_records);
    expect(dbc_create.ok, "Unicode table export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U}
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"ALICE"}});
    expect(table_create.ok, "Unicode table export test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_json(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_json should resolve a Unicode catalog table filename");
    if (result.ok) {
        expect(
            result.json.find("\"ALICE\"") != std::string::npos,
            "export JSON should include rows from a Unicode catalog table filename");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_sql_maps_currency_datetime_and_blank_numeric() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sql_value_mapping_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "accounts.dbf";
    // An embedded CR/LF in the catalog's own DATABASE object name: a crafted
    // DBC controls this value, and export_database_as_sql() must not let it
    // escape the "-- database: ..." comment line into executable SQL.
    const std::string injected_db_name = "Accounts\n'; DROP TABLE accounts; --";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> dbc_records{
        {"DATABASE", injected_db_name, "", ""},
        {"TABLE", "accounts", injected_db_name, ""}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields, dbc_records);
    expect(dbc_create.ok, "SQL value-mapping test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "BALANCE", .type = 'Y', .offset = 1U, .length = 8U, .decimal_count = 0U},
        {.name = "OPENED", .type = 'T', .offset = 9U, .length = 8U, .decimal_count = 0U},
        {.name = "SCORE", .type = 'N', .offset = 17U, .length = 5U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path),
        table_fields,
        {{"123.45", "julian:2459625 millis:37230000", ""}});
    expect(table_create.ok, "SQL value-mapping test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sql should resolve the accounts fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("\r'; DROP TABLE") == std::string::npos &&
               result.sql.find("\n'; DROP TABLE") == std::string::npos,
           "export_database_as_sql must not let an embedded newline escape a -- comment");
    expect(result.sql.find("DROP TABLE accounts") != std::string::npos,
           "the sanitized (space-substituted) injected text should still appear, just harmlessly on the comment line");

    expect(result.sql.find("\"BALANCE\" DECIMAL(19, 4)") != std::string::npos,
           "export_database_as_sql should map a Y (currency) field to a fixed DECIMAL(19, 4), not the raw descriptor length/decimals");
    expect(result.sql.find("123.4500") != std::string::npos,
           "export_database_as_sql should emit the currency value unquoted at its native scale");

    expect(result.sql.find("\"OPENED\" TIMESTAMP") != std::string::npos,
           "export_database_as_sql should still declare a T field as TIMESTAMP");
    expect(result.sql.find("'2024-01-17 10:20:30'") != std::string::npos,
           "export_database_as_sql should convert the internal julian/millis storage contract into a real timestamp literal");
    expect(result.sql.find("julian:") == std::string::npos,
           "export_database_as_sql must never leak the raw internal datetime storage representation into the script");

    // SCORE was written as "" (a blank numeric cell): decodes to an empty
    // display_value, not is_null, so it must become NULL, not an empty
    // string literal, in a column declared DECIMAL/INTEGER.
    expect(result.sql.find("VALUES (123.4500, '2024-01-17 10:20:30', NULL)") != std::string::npos,
           "export_database_as_sql should emit NULL for a blank numeric cell rather than an empty string literal");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_access_sql_maps_currency_datetime_and_dates() {
    // #5475: export_database_as_access_sql() shares export_database_as_sql()'s
    // catalog/table-walking logic but swaps in the Access/Jet SQL dialect --
    // square-bracket identifiers, Access-native column types, and #...#
    // date/time literals -- per docs/66-access-container-format-notes.md's
    // finding that the Access SQL/DDL dialect is citable public
    // documentation even though the physical MDB/ACCDB byte layout is not.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_access_sql_value_mapping_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "accounts.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> dbc_records{
        {"DATABASE", "Accounts", "", ""},
        {"TABLE", "accounts", "Accounts", ""}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields, dbc_records);
    expect(dbc_create.ok, "Access SQL value-mapping test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "BALANCE", .type = 'Y', .offset = 1U, .length = 8U, .decimal_count = 0U},
        {.name = "OPENED", .type = 'T', .offset = 9U, .length = 8U, .decimal_count = 0U},
        {.name = "SINCE", .type = 'D', .offset = 17U, .length = 8U, .decimal_count = 0U},
        {.name = "SCORE", .type = 'N', .offset = 25U, .length = 5U, .decimal_count = 0U},
        {.name = "NAME", .type = 'C', .offset = 30U, .length = 20U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path),
        table_fields,
        {{"123.45", "julian:2459625 millis:37230000", "20240117", "", "Jane"}});
    expect(table_create.ok, "Access SQL value-mapping test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_access_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_access_sql should resolve the accounts fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("CREATE TABLE [accounts]") != std::string::npos,
           "export_database_as_access_sql should quote identifiers with square brackets, not double quotes");
    expect(result.sql.find("[BALANCE] CURRENCY") != std::string::npos,
           "export_database_as_access_sql should map a Y (currency) field to Access's native CURRENCY type");
    expect(result.sql.find("[OPENED] DATETIME") != std::string::npos,
           "export_database_as_access_sql should map a T field to Access's DATETIME type");
    expect(result.sql.find("[SINCE] DATETIME") != std::string::npos,
           "export_database_as_access_sql should map a D field to Access's DATETIME type too (no separate date-only type)");
    expect(result.sql.find("[NAME] TEXT(20)") != std::string::npos,
           "export_database_as_access_sql should map a short Character field to TEXT(length)");

    expect(result.sql.find("#2024-01-17 10:20:30#") != std::string::npos,
           "export_database_as_access_sql should emit a T value as a #...#-delimited literal, not quoted");
    expect(result.sql.find("#2024-01-17#") != std::string::npos,
           "export_database_as_access_sql should emit a D value as a #...#-delimited literal");
    expect(result.sql.find("julian:") == std::string::npos,
           "export_database_as_access_sql must never leak the raw internal datetime storage representation");

    // SCORE was written as "" (a blank numeric cell): must still be NULL,
    // matching export_database_as_sql()'s own established behavior.
    expect(result.sql.find(
               "VALUES (123.4500, #2024-01-17 10:20:30#, #2024-01-17#, NULL, 'Jane');") != std::string::npos,
           "export_database_as_access_sql should emit NULL for a blank numeric cell rather than an empty string literal");

    // #5475 review (Codex): independently verified that native Jet/ACE SQL
    // -- whether run through Access's interactive SQL View or a DAO/ADO
    // Execute() call -- has no supported comment syntax at all, "--"
    // included. A script advertised as directly runnable against a real
    // Access database must not embed one, or it would fail exactly where
    // this exporter's whole purpose is to succeed.
    expect(result.sql.find("--") == std::string::npos,
           "export_database_as_access_sql must not emit any '--' comment; Jet/ACE SQL has no comment syntax to run one through");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_access_sql_escapes_bracket_in_identifier() {
    // #5475 review (Copilot): a crafted/corrupt DBC's catalog is not bound
    // by Access's own UI naming restrictions, so a table or field name
    // containing ']' must be escaped (by doubling, the Jet/ACE bracket-
    // escape convention), not assumed impossible -- otherwise it would
    // break out of the [identifier] quoting and inject arbitrary SQL.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_access_sql_bracket_escape_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::string crafted_table_name = "acct]; DROP TABLE [x";
    // load_database_catalog_snapshot() resolves a table's physical path
    // by appending ".dbf" to the catalog's own object name, so the
    // on-disk fixture must be named to match the crafted name exactly,
    // not a plain "accounts.dbf" the catalog doesn't actually reference.
    const fs::path table_path = temp_dir / (crafted_table_name + ".dbf");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path),
        dbc_fields,
        {{"DATABASE", "Accounts", "", ""}, {"TABLE", crafted_table_name, "Accounts", ""}});
    expect(dbc_create.ok, "Access SQL bracket-escape test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"Jane"}});
    expect(table_create.ok, "Access SQL bracket-escape test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_access_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_access_sql should resolve the crafted-name fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("CREATE TABLE [acct]]; DROP TABLE [x] (") != std::string::npos,
           "export_database_as_access_sql should escape an embedded ']' by doubling it, not leave it unescaped");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_access_sql_clamps_decimal_precision() {
    // #5475 review (Copilot): a crafted/invalid DBF header can have
    // decimal_count exceed length, and Access's own DECIMAL type caps
    // precision at 28 -- both must be clamped into valid ranges rather
    // than trusted, or the generated DDL is invalid and Access rejects it.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_access_sql_decimal_clamp_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "readings.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path),
        dbc_fields,
        {{"DATABASE", "Sensors", "", ""}, {"TABLE", "readings", "Sensors", ""}});
    expect(dbc_create.ok, "Access SQL decimal-clamp test: DBC fixture should be created");

    // create_dbf_table_file() itself refuses to create a field with
    // decimal_count > length (a well-formed writer never produces one),
    // so a genuinely invalid header -- modeling a crafted/foreign source
    // this codebase did not write -- has to be synthesized by patching
    // the raw descriptor byte after creation, not requested through the
    // public API directly.
    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "VALUE", .type = 'N', .offset = 1U, .length = 5U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"1.23"}});
    expect(table_create.ok, "Access SQL decimal-clamp test: DBF fixture should be created");
    {
        // Field descriptors start at byte 32 in this writer's layout, one
        // per field, 32 bytes each; the decimal-count byte is at offset
        // 17 within its own descriptor (dbf_table.cpp's own
        // descriptor_decimal_count_offset). VALUE is the only field, so
        // its descriptor starts at byte 32.
        std::fstream patch(table_path, std::ios::binary | std::ios::in | std::ios::out);
        expect(static_cast<bool>(patch), "Access SQL decimal-clamp test: DBF fixture should be patchable");
        patch.seekp(32 + 17, std::ios::beg);
        const char corrupted_decimal_count = static_cast<char>(9);
        patch.write(&corrupted_decimal_count, 1);
    }

    const auto result = copperfin::vfp::export_database_as_access_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_access_sql should resolve the clamp-test fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("[VALUE] DECIMAL(5, 5)") != std::string::npos,
           "export_database_as_access_sql should clamp scale down to precision when decimal_count exceeds length");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_access_sql_rejects_unsafe_numeric_token() {
    // #5475 review (Codex): a numeric field's decoded display_value is
    // emitted unquoted (a real numeric literal needs no string
    // delimiters), so unlike a string value there is no quoting layer to
    // escape a crafted/corrupted value with. An overflow marker like
    // "*****" (dBASE-family's own convention for a too-wide value) must
    // become NULL, not be trusted verbatim into the generated SQL.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_access_sql_numeric_safety_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "readings.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path),
        dbc_fields,
        {{"DATABASE", "Sensors", "", ""}, {"TABLE", "readings", "Sensors", ""}});
    expect(dbc_create.ok, "Access SQL numeric-safety test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "VALUE", .type = 'N', .offset = 1U, .length = 5U, .decimal_count = 0U},
    };
    // "*****" is the classic dBASE-family numeric-overflow fill, written
    // directly via create_dbf_table_file()'s allow_truncation path by
    // matching the field's exact width so it round-trips as literal text
    // rather than being rejected as too-wide at write time.
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"*****"}});
    expect(table_create.ok, "Access SQL numeric-safety test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_access_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_access_sql should resolve the numeric-safety fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("VALUES (NULL);") != std::string::npos,
           "export_database_as_access_sql should emit NULL for an overflow-marker/non-numeric cell rather than trust it unquoted");
    expect(result.sql.find("*****") == std::string::npos,
           "export_database_as_access_sql must never emit an unsafe unquoted numeric token verbatim");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_access_sql_accepts_leading_plus_sign_numeric() {
    // #5475 re-review (Copilot): looks_like_safe_unquoted_sql_numeric_literal()
    // initially accepted only a leading '-', not '+' -- but this codebase's
    // own value parsing elsewhere (e.g. parse_scaled_currency_value(),
    // dbf_table.cpp) already treats a leading '+' as valid numeric input,
    // so a genuinely real "+42" value must still round-trip unquoted, not
    // be misclassified as unsafe and silently turned into NULL.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_access_sql_plus_sign_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "readings.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path),
        dbc_fields,
        {{"DATABASE", "Sensors", "", ""}, {"TABLE", "readings", "Sensors", ""}});
    expect(dbc_create.ok, "Access SQL plus-sign test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "VALUE", .type = 'N', .offset = 1U, .length = 5U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"+42"}});
    expect(table_create.ok, "Access SQL plus-sign test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_access_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_access_sql should resolve the plus-sign fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("VALUES (+42);") != std::string::npos,
           "export_database_as_access_sql should emit a leading-'+' numeric value unquoted, not NULL");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_access_sql_rejects_unsafe_date_literal() {
    // #5475 re-review (Copilot): the 'D' field branch embedded
    // display_value directly inside #...# delimiters. decode_value()'s
    // 'D' case (dbf_table.cpp) assembles "YYYY-MM-DD" by slicing raw
    // bytes at fixed positions without validating they're digits, so a
    // corrupt/crafted date field can smuggle a literal '#' straight into
    // the formatted value and break out of the delimiter. Corrupt one
    // byte of an otherwise well-formed 8-byte date field directly (the
    // public writer always produces genuinely valid digits, so this
    // models a source Copperfin did not write) and confirm the exporter
    // now falls back to NULL instead of embedding it.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_access_sql_date_injection_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "events.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path),
        dbc_fields,
        {{"DATABASE", "Events", "", ""}, {"TABLE", "events", "Events", ""}});
    expect(dbc_create.ok, "Access SQL date-injection test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "OCCURRED", .type = 'D', .offset = 1U, .length = 8U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"20240117"}});
    expect(table_create.ok, "Access SQL date-injection test: DBF fixture should be created");
    {
        // Single field, single record: header_length = 32 (header) +
        // 32 (one descriptor) + 1 (terminator) = 65; the field's own
        // offset within a record is 1 (byte 0 is the deletion flag), so
        // the 8-byte date value starts at absolute byte 66. Corrupt its
        // 5th byte (the "MM" tens digit) to '#'.
        std::fstream patch(table_path, std::ios::binary | std::ios::in | std::ios::out);
        expect(static_cast<bool>(patch), "Access SQL date-injection test: DBF fixture should be patchable");
        patch.seekp(66 + 4, std::ios::beg);
        const char injected_hash = '#';
        patch.write(&injected_hash, 1);
    }

    const auto result = copperfin::vfp::export_database_as_access_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_access_sql should resolve the date-injection fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("VALUES (NULL);") != std::string::npos,
           "export_database_as_access_sql should emit NULL for a corrupt date value rather than embed it unvalidated");
    expect(result.sql.find("2024-#1-17") == std::string::npos,
           "export_database_as_access_sql must never let an injected '#' break out of the date-literal delimiter");

    fs::remove_all(temp_dir, ignored);
}

void test_database_json_import_plan_admits_exporter_unreadable_table_marker() {
    namespace fs = std::filesystem;
    const fs::path temp_dir =
        fs::temp_directory_path() / "copperfin_dbc_unreadable_table_export_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 81U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        dbc_path.string(),
        dbc_fields,
        {{"DATABASE", "UnreadableRuntime", "", ""},
         {"TABLE", "Unreadable", "UnreadableRuntime", ""}});
    expect(dbc_create.ok, "unreadable table export test: DBC fixture should be created");

    {
        std::ofstream unreadable_table(temp_dir / "Unreadable.dbf", std::ios::binary);
        unreadable_table << "not a DBF";
    }

    const auto exported = copperfin::vfp::export_database_as_json(dbc_path.string());
    expect(exported.ok,
           "unreadable table export test: exporter should preserve an unreadable cataloged table");
    if (exported.ok) {
        expect(exported.json.find("\"Unreadable\": {\"fields\":[], \"records\":[]}") != std::string::npos,
               "unreadable table export test: exporter should emit its documented empty-field marker");
        const auto plan = copperfin::vfp::build_database_json_import_plan(exported.json);
        expect(plan.ok && plan.plan.tables.size() == 1U &&
                   plan.plan.tables.front().name == "Unreadable" &&
                   plan.plan.tables.front().fields.empty() &&
                   plan.plan.tables.front().records_json == "[]",
               "database JSON planning should admit the exporter\'s unreadable-table marker without reconstruction");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_parse_real_vfp_cdx_when_available() {
    const std::filesystem::path sample_path =
        "C:\\Program Files (x86)\\Microsoft Visual FoxPro 9\\Samples\\Tastrade\\Data\\customer.cdx";
    if (!std::filesystem::exists(sample_path)) {
        return;
    }

    const auto result = copperfin::vfp::parse_index_probe_from_file(sample_path.string());
    expect(result.ok, "real VFP customer.cdx should parse as a CDX-family index");
    expect(result.probe.kind == copperfin::vfp::IndexKind::cdx, "real VFP customer.cdx should stay typed as CDX");
    expect(!result.probe.tags.empty(), "real VFP customer.cdx should expose at least one parsed tag");

    bool saw_customer_id = false;
    bool saw_company_name = false;
    bool saw_customer_tag = false;
    bool saw_company_tag = false;
    for (const auto& tag : result.probe.tags) {
        saw_customer_id = saw_customer_id || tag.key_expression_hint == "customer_id";
        saw_company_name = saw_company_name || tag.key_expression_hint == "UPPER(company_name)";
        saw_customer_tag = saw_customer_tag || tag.name_hint == "CUSTOMER_I";
        saw_company_tag = saw_company_tag || tag.name_hint == "COMPANY_NA";
    }

    expect(saw_customer_id, "real VFP customer.cdx should expose the customer_id expression");
    expect(saw_company_name, "real VFP customer.cdx should expose the UPPER(company_name) expression");
    expect(saw_customer_tag, "real VFP customer.cdx should expose the CUSTOMER_I tag from directory pages");
    expect(saw_company_tag, "real VFP customer.cdx should expose the COMPANY_NA tag from directory pages");
}

void test_parse_additional_real_vfp_cdx_samples_when_available() {
    const std::vector<std::filesystem::path> sample_paths{
        "C:\\Program Files (x86)\\Microsoft Visual FoxPro 9\\Samples\\Tastrade\\Data\\Orders.CDX",
        "C:\\Program Files (x86)\\Microsoft Visual FoxPro 9\\Samples\\Northwind\\products.cdx",
        "C:\\Program Files (x86)\\Microsoft Visual FoxPro 9\\Samples\\Northwind\\orderdetails.cdx"
    };

    for (const auto& sample_path : sample_paths) {
        if (!std::filesystem::exists(sample_path)) {
            continue;
        }

        const auto result = copperfin::vfp::parse_index_probe_from_file(sample_path.string());
        expect(result.ok, "additional real VFP CDX samples should parse as CDX-family indexes");
        expect(result.probe.kind == copperfin::vfp::IndexKind::cdx, "additional real VFP CDX samples should stay typed as CDX");
        expect(!result.probe.tags.empty(), "additional real VFP CDX samples should expose at least one parsed tag");
        if (!result.probe.tags.empty()) {
            const bool saw_named_or_expression_tag = std::any_of(
                result.probe.tags.begin(),
                result.probe.tags.end(),
                [](const copperfin::vfp::IndexTagProbe& tag) {
                    return !tag.name_hint.empty() || !tag.key_expression_hint.empty();
                });
            expect(saw_named_or_expression_tag, "additional real VFP CDX samples should expose at least one named or expression-backed tag");
        }
    }
}

void test_parse_real_vfp_dcx_samples_when_available() {
    const std::vector<std::filesystem::path> sample_paths{
        "C:\\Program Files (x86)\\Microsoft Visual FoxPro 9\\Samples\\Tastrade\\Data\\tastrade.dcx",
        "C:\\Program Files (x86)\\Microsoft Visual FoxPro 9\\Samples\\Northwind\\northwind.dcx"
    };

    for (const auto& sample_path : sample_paths) {
        if (!std::filesystem::exists(sample_path)) {
            continue;
        }

        const auto result = copperfin::vfp::parse_index_probe_from_file(sample_path.string());
        expect(result.ok, "real VFP DCX samples should parse as DCX-family indexes");
        expect(result.probe.kind == copperfin::vfp::IndexKind::dcx, "real VFP DCX samples should stay typed as DCX");
        expect(!result.probe.tags.empty(), "real VFP DCX samples should expose at least one parsed tag");
    }
}

void test_parse_real_dbase_ndx_when_available() {
    const std::filesystem::path sample_path = "E:\\DBASE\\DBFS\\CHNGREAS.NDX";
    if (!std::filesystem::exists(sample_path)) {
        return;
    }

    const auto result = copperfin::vfp::parse_index_probe_from_file(sample_path.string());
    expect(result.ok, "real dBase NDX samples should parse as NDX-family indexes");
    expect(result.probe.kind == copperfin::vfp::IndexKind::ndx, "real dBase NDX samples should stay typed as NDX");
    expect(result.probe.root_node_offset_hint >= 512U, "real dBase NDX samples should expose a plausible root-node offset");
    expect(!result.probe.key_expression_hint.empty(), "real dBase NDX samples should expose a key expression hint");
}

void test_inspect_asset_reports_dbf_storage_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_asset_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path short_header_path = temp_dir / "short_header.dbf";
    std::vector<std::uint8_t> short_header_bytes(32U, 0U);
    short_header_bytes[0] = 0x30U;
    write_le_u32(short_header_bytes, 4U, 1U);
    write_le_u16(short_header_bytes, 8U, 97U);
    write_le_u16(short_header_bytes, 10U, 13U);
    {
        std::ofstream output(short_header_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(short_header_bytes.data()), static_cast<std::streamsize>(short_header_bytes.size()));
    }

    const auto short_header_result = copperfin::vfp::inspect_asset(short_header_path.string());
    expect(short_header_result.ok, "inspect_asset should still succeed when a DBF header can be parsed but the file is structurally short");
    expect(short_header_result.header_available, "header metadata should still be available for a DBF with validation findings");
    expect(
        has_validation_issue(short_header_result, "dbf.header_length_exceeds_file_size", "short_header.dbf"),
        "inspect_asset should report when the DBF header length exceeds the file size");
    const auto* short_header_issue =
        find_validation_issue(short_header_result, "dbf.header_length_exceeds_file_size", "short_header.dbf");
    expect(
        short_header_issue != nullptr &&
            short_header_issue->severity == copperfin::vfp::AssetValidationSeverity::error &&
            short_header_issue->message == "The DBF header length exceeds the file size.",
        "#2387: DBF storage validation output should preserve code, severity, and default message text");

    const fs::path truncated_records_path = temp_dir / "truncated_records.dbf";
    std::vector<std::uint8_t> truncated_bytes(100U, 0U);
    truncated_bytes[0] = 0x30U;
    write_le_u32(truncated_bytes, 4U, 3U);
    write_le_u16(truncated_bytes, 8U, 65U);
    write_le_u16(truncated_bytes, 10U, 16U);
    {
        std::ofstream output(truncated_records_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(truncated_bytes.data()), static_cast<std::streamsize>(truncated_bytes.size()));
    }

    const auto truncated_result = copperfin::vfp::inspect_asset(truncated_records_path.string());
    expect(truncated_result.ok, "inspect_asset should still succeed for DBFs with truncated record storage");
    expect(
        has_validation_issue(truncated_result, "dbf.record_storage_truncated", "truncated_records.dbf"),
        "inspect_asset should report truncated DBF record storage");

    fs::remove_all(temp_dir, ignored);
}

void test_inspect_asset_reports_missing_companions_and_unparseable_indexes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_asset_companion_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "missing_sidecar.scx";
    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto form_result = copperfin::vfp::inspect_asset(form_path.string());
    expect(form_result.ok, "inspect_asset should succeed for a readable SCX even when its sidecar is missing");
    expect(
        has_validation_issue(form_result, "memo.sidecar_missing", "missing_sidecar.sct"),
        "inspect_asset should report a missing SCX memo sidecar");
    const auto* missing_sidecar_issue =
        find_validation_issue(form_result, "memo.sidecar_missing", "missing_sidecar.sct");
    expect(
        missing_sidecar_issue != nullptr &&
            missing_sidecar_issue->severity == copperfin::vfp::AssetValidationSeverity::error &&
            missing_sidecar_issue->message ==
                "The DBF-family asset expects a memo sidecar file, but the sidecar is missing.",
        "#2387: memo sidecar validation output should preserve code, severity, and default message text");

    const fs::path label_path = temp_dir / "missing_sidecar.lbx";
    {
        const auto bytes = make_vfp_header();
        std::ofstream output(label_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto label_result = copperfin::vfp::inspect_asset(label_path.string());
    expect(label_result.ok, "inspect_asset should succeed for a readable LBX even when its sidecar is missing");
    expect(
        has_validation_issue(label_result, "memo.sidecar_missing", "missing_sidecar.lbt"),
        "#3215: inspect_asset should report the LBX memo sidecar path through the label family route");
    const auto* missing_label_sidecar_issue =
        find_validation_issue(label_result, "memo.sidecar_missing", "missing_sidecar.lbt");
    expect(
        missing_label_sidecar_issue != nullptr &&
            missing_label_sidecar_issue->severity == copperfin::vfp::AssetValidationSeverity::error &&
            missing_label_sidecar_issue->message ==
                "The DBF-family asset expects a memo sidecar file, but the sidecar is missing.",
        "#3215: label-family memo sidecar validation should stay aligned with other DBF-family assets");

    const fs::path table_path = temp_dir / "broken_index.dbf";
    const fs::path bad_cdx_path = temp_dir / "broken_index.cdx";
    {
        auto bytes = make_vfp_header();
        bytes[28] = 0x00U;
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }
    {
        std::ofstream output(bad_cdx_path, std::ios::binary);
        output << "not a real index";
    }

    const auto table_result = copperfin::vfp::inspect_asset(table_path.string());
    expect(table_result.ok, "inspect_asset should succeed for a readable DBF even when a companion index is malformed");
    expect(
        has_validation_issue(table_result, "index.companion_parse_failed", "broken_index.cdx"),
        "inspect_asset should report malformed companion indexes as structured validation findings");
    const auto* bad_index_issue =
        find_validation_issue(table_result, "index.companion_parse_failed", "broken_index.cdx");
    expect(
        bad_index_issue != nullptr &&
            bad_index_issue->severity == copperfin::vfp::AssetValidationSeverity::warning &&
            bad_index_issue->message.starts_with("A companion index file exists but could not be parsed: "),
        "#2387: companion index validation output should preserve placeholders and invariant severity");

    const fs::path indexed_table_path = temp_dir / "missing_structural_index.dbf";
    {
        auto bytes = make_vfp_header();
        bytes[28] = 0x01U;
        std::ofstream output(indexed_table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto indexed_table_result = copperfin::vfp::inspect_asset(indexed_table_path.string());
    expect(indexed_table_result.ok, "inspect_asset should succeed for readable DBFs even when the structural index sidecar is missing");
    expect(
        has_validation_issue(indexed_table_result, "index.structural_sidecar_missing", "missing_structural_index.dbf"),
        "inspect_asset should report a missing structural index companion when the DBF production-index flag is set");

    const fs::path dbc_path = temp_dir / "missing_database_sidecars.dbc";
    {
        const auto bytes = make_vfp_header();
        std::ofstream output(dbc_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto dbc_result = copperfin::vfp::inspect_asset(dbc_path.string());
    expect(dbc_result.ok, "inspect_asset should succeed for a readable DBC even when companion files are missing");
    expect(
        has_validation_issue(dbc_result, "memo.sidecar_missing", "missing_database_sidecars.dct"),
        "inspect_asset should report a missing DBC memo sidecar");
    expect(
        has_validation_issue(dbc_result, "index.structural_sidecar_missing", "missing_database_sidecars.dbc"),
        "inspect_asset should report a missing DBC structural index companion");

    fs::remove_all(temp_dir, ignored);
}

void test_inspect_asset_reports_malformed_memo_sidecar_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_asset_memo_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto write_form_with_memo_fields = [&](
        const fs::path& table_path,
        const std::vector<char>& field_types,
        const std::vector<std::vector<std::uint32_t>>& record_pointers) {
        const std::size_t header_length = 33U + (field_types.size() * 32U);
        const std::size_t record_length = 1U + (field_types.size() * 4U);
        std::vector<std::uint8_t> table_bytes(
            header_length + (record_pointers.size() * record_length) + 1U,
            0U);
        table_bytes[0] = 0x30U;
        table_bytes[1] = 126U;
        table_bytes[2] = 4U;
        table_bytes[3] = 11U;
        write_le_u32(table_bytes, 4U, static_cast<std::uint32_t>(record_pointers.size()));
        write_le_u16(table_bytes, 8U, static_cast<std::uint16_t>(header_length));
        write_le_u16(table_bytes, 10U, static_cast<std::uint16_t>(record_length));
        table_bytes[28U] = 0x00U;
        table_bytes[29U] = 0x03U;
        for (std::size_t field_index = 0U; field_index < field_types.size(); ++field_index) {
            write_field_descriptor(
                table_bytes,
                32U + (field_index * 32U),
                "MEMO" + std::to_string(field_index + 1U),
                field_types[field_index],
                static_cast<std::uint32_t>(1U + (field_index * 4U)),
                4U);
        }
        table_bytes[header_length - 1U] = 0x0DU;
        for (std::size_t record_index = 0U; record_index < record_pointers.size(); ++record_index) {
            expect(
                record_pointers[record_index].size() == field_types.size(),
                "#3983: memo validation fixture records should match the field count");
            const std::size_t record_offset = header_length + (record_index * record_length);
            table_bytes[record_offset] = 0x20U;
            const std::size_t pointer_count =
                std::min(record_pointers[record_index].size(), field_types.size());
            for (std::size_t field_index = 0U; field_index < pointer_count; ++field_index) {
                write_le_u32(
                    table_bytes,
                    record_offset + 1U + (field_index * 4U),
                    record_pointers[record_index][field_index]);
            }
        }
        table_bytes.back() = 0x1AU;

        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    };
    const auto write_form_with_memo_pointer = [&](
        const fs::path& table_path,
        std::uint32_t block_pointer) {
        write_form_with_memo_fields(table_path, {'M'}, {{block_pointer}});
    };

    const fs::path bad_header_form_path = temp_dir / "bad_header.scx";
    const fs::path bad_header_sidecar_path = temp_dir / "bad_header.sct";
    write_form_with_memo_pointer(bad_header_form_path, 1U);
    {
        std::vector<std::uint8_t> memo_bytes(32U, 0U);
        write_be_u16(memo_bytes, 6U, 0U);
        std::ofstream output(bad_header_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const auto bad_header_result = copperfin::vfp::inspect_asset(bad_header_form_path.string());
    expect(bad_header_result.ok, "inspect_asset should still succeed for forms with malformed memo sidecar headers");
    expect(
        has_validation_issue(bad_header_result, "memo.sidecar_shorter_than_block_size", "bad_header.sct") ||
        has_validation_issue(bad_header_result, "memo.block_size_invalid", "bad_header.sct") ||
        has_validation_issue(bad_header_result, "memo.sidecar_header_truncated", "bad_header.sct"),
        "inspect_asset should report malformed memo sidecar header metadata");

    const fs::path out_of_range_form_path = temp_dir / "pointer_out_of_range.scx";
    const fs::path out_of_range_sidecar_path = temp_dir / "pointer_out_of_range.sct";
    write_form_with_memo_pointer(out_of_range_form_path, 3U);
    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        std::ofstream output(out_of_range_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const auto out_of_range_result = copperfin::vfp::inspect_asset(out_of_range_form_path.string());
    expect(out_of_range_result.ok, "inspect_asset should still succeed for forms with out-of-range memo pointers");
    expect(
        has_validation_issue(out_of_range_result, "memo.pointer_out_of_range", "pointer_out_of_range.sct"),
        "inspect_asset should report memo pointers that fall outside the sidecar range");
    const auto* pointer_issue =
        find_validation_issue(out_of_range_result, "memo.pointer_out_of_range", "pointer_out_of_range.sct");
    expect(
        pointer_issue != nullptr &&
            pointer_issue->message == "A memo field points to a block outside the available sidecar range.",
        "#2387: memo pointer validation output should preserve default message text");

    const fs::path truncated_form_path = temp_dir / "payload_truncated.scx";
    const fs::path truncated_sidecar_path = temp_dir / "payload_truncated.sct";
    write_form_with_memo_pointer(truncated_form_path, 1U);
    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        memo_bytes[512U + 3U] = 1U;
        write_be_u32(memo_bytes, 512U + 4U, 900U);
        std::ofstream output(truncated_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const auto truncated_result = copperfin::vfp::inspect_asset(truncated_form_path.string());
    expect(truncated_result.ok, "inspect_asset should still succeed for forms with truncated memo payloads");
    expect(
        has_validation_issue(truncated_result, "memo.payload_truncated", "payload_truncated.sct"),
        "inspect_asset should report truncated payloads in referenced memo blocks");

    const fs::path general_form_path = temp_dir / "general_pointer_out_of_range.scx";
    const fs::path general_sidecar_path = temp_dir / "general_pointer_out_of_range.sct";
    write_form_with_memo_fields(general_form_path, {'G'}, {{3U}});
    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        std::ofstream output(general_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
    const auto general_result = copperfin::vfp::inspect_asset(general_form_path.string());
    expect(
        has_validation_issue(general_result, "memo.pointer_out_of_range", "general_pointer_out_of_range.sct"),
        "#3983: inspect_asset should validate General-field memo pointers");

    const fs::path picture_form_path = temp_dir / "picture_payload_truncated.scx";
    const fs::path picture_sidecar_path = temp_dir / "picture_payload_truncated.sct";
    write_form_with_memo_fields(picture_form_path, {'P'}, {{1U}});
    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        write_be_u32(memo_bytes, 512U, 1U);
        write_be_u32(memo_bytes, 516U, 900U);
        std::ofstream output(picture_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
    const auto picture_result = copperfin::vfp::inspect_asset(picture_form_path.string());
    expect(
        has_validation_issue(picture_result, "memo.payload_truncated", "picture_payload_truncated.sct"),
        "#3983: inspect_asset should validate Picture-field memo payload bounds");

    const fs::path internal_terminator_byte_form_path =
        temp_dir / "descriptor_contains_terminator_byte.scx";
    const fs::path internal_terminator_byte_sidecar_path =
        temp_dir / "descriptor_contains_terminator_byte.sct";
    {
        std::vector<std::uint8_t> table_bytes(115U, 0U);
        table_bytes[0] = 0x30U;
        write_le_u32(table_bytes, 4U, 1U);
        write_le_u16(table_bytes, 8U, 97U);
        write_le_u16(table_bytes, 10U, 17U);
        write_field_descriptor(table_bytes, 32U, "OBJNAME", 'C', 1U, 12U);
        write_field_descriptor(table_bytes, 64U, "PROPERTIES", 'M', 13U, 4U);
        table_bytes[96U] = 0x0DU;
        table_bytes[97U] = 0x20U;
        write_ascii(table_bytes, 98U, "txtTitle");
        write_le_u32(table_bytes, 110U, 3U);
        table_bytes.back() = 0x1AU;
        std::ofstream output(internal_terminator_byte_form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        std::ofstream output(internal_terminator_byte_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
    const auto internal_terminator_byte_result =
        copperfin::vfp::inspect_asset(internal_terminator_byte_form_path.string());
    expect(
        !has_validation_issue(internal_terminator_byte_result, "dbf.descriptor_span_misaligned"),
        "#3983: offset 13 inside a valid descriptor must not become a false terminator");
    expect(
        has_validation_issue(
            internal_terminator_byte_result,
            "memo.pointer_out_of_range",
            "descriptor_contains_terminator_byte.sct"),
        "#3983: aligned terminator discovery should still validate memo fields after an internal 0x0D byte");

    const fs::path multiple_form_path = temp_dir / "multiple_memo_fields.scx";
    const fs::path multiple_sidecar_path = temp_dir / "multiple_memo_fields.sct";
    write_form_with_memo_fields(
        multiple_form_path,
        {'M', 'G', 'P'},
        {
            {0U, 3U, 3U},
            {1U, 1U, 2U}
        });
    {
        std::vector<std::uint8_t> memo_bytes(1536U, 0U);
        write_be_u32(memo_bytes, 0U, 3U);
        write_be_u16(memo_bytes, 6U, 512U);
        write_be_u32(memo_bytes, 512U, 1U);
        write_be_u32(memo_bytes, 516U, 4U);
        write_ascii(memo_bytes, 520U, "safe");
        write_be_u32(memo_bytes, 1024U, 1U);
        write_be_u32(memo_bytes, 1028U, 900U);
        std::ofstream output(multiple_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
    const auto multiple_result = copperfin::vfp::inspect_asset(multiple_form_path.string());
    expect(
        count_validation_issues(multiple_result, "memo.pointer_out_of_range") == 1U,
        "#3983: shared corrupt blocks across later memo fields should produce one pointer diagnostic");
    expect(
        count_validation_issues(multiple_result, "memo.payload_truncated") == 1U,
        "#3983: corruption in a later record and Picture field should be validated once");

    const fs::path missing_terminator_form_path = temp_dir / "memo_descriptor_terminator_missing.scx";
    const fs::path missing_terminator_sidecar_path = temp_dir / "memo_descriptor_terminator_missing.sct";
    {
        std::vector<std::uint8_t> table_bytes(98U, 0U);
        table_bytes[0] = 0x30U;
        write_le_u32(table_bytes, 4U, 1U);
        write_le_u16(table_bytes, 8U, 65U);
        write_le_u16(table_bytes, 10U, 32U);
        write_field_descriptor(table_bytes, 32U, "NAME", 'C', 1U, 4U);
        table_bytes[65U] = 0x20U;
        write_le_u32(table_bytes, 66U, 3U);
        table_bytes[75U] = static_cast<std::uint8_t>('M');
        write_le_u32(table_bytes, 76U, 1U);
        table_bytes[80U] = 4U;
        table_bytes.back() = 0x1AU;
        std::ofstream output(missing_terminator_form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        std::ofstream output(missing_terminator_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
    const auto missing_terminator_result =
        copperfin::vfp::inspect_asset(missing_terminator_form_path.string());
    expect(
        has_validation_issue(missing_terminator_result, "dbf.descriptor_terminator_missing"),
        "#3983: malformed descriptor fixtures should retain their descriptor diagnostic");
    expect(
        !has_validation_issue(missing_terminator_result, "memo.pointer_out_of_range") &&
            !has_validation_issue(missing_terminator_result, "memo.payload_truncated"),
        "#3983: record bytes after a missing descriptor terminator must not become fake memo fields");

    const fs::path zero_offset_form_path = temp_dir / "memo_field_offset_zero.scx";
    const fs::path zero_offset_sidecar_path = temp_dir / "memo_field_offset_zero.sct";
    {
        std::vector<std::uint8_t> table_bytes(71U, 0U);
        table_bytes[0] = 0x30U;
        write_le_u32(table_bytes, 4U, 1U);
        write_le_u16(table_bytes, 8U, 65U);
        write_le_u16(table_bytes, 10U, 5U);
        write_field_descriptor(table_bytes, 32U, "MEMO1", 'M', 0U, 4U);
        table_bytes[64U] = 0x0DU;
        write_le_u32(table_bytes, 65U, 3U);
        table_bytes.back() = 0x1AU;
        std::ofstream output(zero_offset_form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        std::ofstream output(zero_offset_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
    const auto zero_offset_result = copperfin::vfp::inspect_asset(zero_offset_form_path.string());
    expect(
        has_validation_issue(zero_offset_result, "dbf.field_offset_invalid"),
        "#3983: zero-offset memo fields should retain the descriptor-offset diagnostic");
    expect(
        !has_validation_issue(zero_offset_result, "memo.pointer_out_of_range") &&
            !has_validation_issue(zero_offset_result, "memo.payload_truncated"),
        "#3983: a zero-offset memo field must not reinterpret the deletion flag as a pointer");

    const fs::path clean_form_path = temp_dir / "clean_memo_fields.scx";
    const fs::path clean_sidecar_path = temp_dir / "clean_memo_fields.sct";
    write_form_with_memo_fields(
        clean_form_path,
        {'M', 'G', 'P'},
        {
            {0U, 1U, 1U},
            {1U, 0U, 1U}
        });
    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        write_be_u32(memo_bytes, 512U, 1U);
        write_be_u32(memo_bytes, 516U, 4U);
        write_ascii(memo_bytes, 520U, "safe");
        std::ofstream output(clean_sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
    const auto clean_result = copperfin::vfp::inspect_asset(clean_form_path.string());
    expect(
        !has_validation_issue(clean_result, "memo.pointer_out_of_range") &&
            !has_validation_issue(clean_result, "memo.payload_truncated"),
        "#3983: zero pointers and valid shared M/G/P blocks should remain clean");

    fs::remove_all(temp_dir, ignored);
}

void test_inspect_asset_reports_dbf_descriptor_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_descriptor_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path missing_terminator_path = temp_dir / "missing_terminator.dbf";
    {
        std::vector<std::uint8_t> bytes(97U, 0U);
        bytes[0] = 0x30U;
        write_le_u32(bytes, 4U, 1U);
        write_le_u16(bytes, 8U, 97U);
        write_le_u16(bytes, 10U, 16U);
        write_field_descriptor(bytes, 32U, "NAME", 'C', 1U, 10U);
        bytes[96U] = 0x20U;
        std::ofstream output(missing_terminator_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto missing_terminator_result = copperfin::vfp::inspect_asset(missing_terminator_path.string());
    expect(missing_terminator_result.ok, "inspect_asset should still succeed for DBFs with descriptor validation findings");
    expect(
        has_validation_issue(missing_terminator_result, "dbf.descriptor_terminator_missing", "missing_terminator.dbf"),
        "inspect_asset should report missing DBF descriptor terminators");

    const fs::path record_layout_path = temp_dir / "record_layout.dbf";
    {
        std::vector<std::uint8_t> bytes(129U, 0U);
        bytes[0] = 0x30U;
        write_le_u32(bytes, 4U, 1U);
        write_le_u16(bytes, 8U, 97U);
        write_le_u16(bytes, 10U, 12U);
        write_field_descriptor(bytes, 32U, "FIRST", 'C', 1U, 10U);
        write_field_descriptor(bytes, 64U, "SECOND", 'C', 8U, 8U);
        bytes[96U] = 0x0DU;
        bytes[97U] = 0x20U;
        bytes[128U] = 0x1AU;
        std::ofstream output(record_layout_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto record_layout_result = copperfin::vfp::inspect_asset(record_layout_path.string());
    expect(record_layout_result.ok, "inspect_asset should still succeed for DBFs with bad field layout metadata");
    expect(
        has_validation_issue(record_layout_result, "dbf.field_layout_overlap", "record_layout.dbf"),
        "inspect_asset should report overlapping DBF field descriptors");
    const auto* overlap_issue =
        find_validation_issue(record_layout_result, "dbf.field_layout_overlap", "record_layout.dbf");
    expect(
        overlap_issue != nullptr &&
            overlap_issue->message == "The DBF contains overlapping field descriptors.",
        "#2387: descriptor validation output should preserve default message text");
    expect(
        has_validation_issue(record_layout_result, "dbf.field_layout_overflow", "record_layout.dbf"),
        "inspect_asset should report field descriptors that overflow the declared record length");
    expect(
        has_validation_issue(record_layout_result, "dbf.record_length_mismatch", "record_layout.dbf"),
        "inspect_asset should report descriptor-derived record length mismatches");

    const fs::path field_names_path = temp_dir / "field_names.dbf";
    {
        std::vector<std::uint8_t> bytes(129U, 0U);
        bytes[0] = 0x30U;
        write_le_u32(bytes, 4U, 1U);
        write_le_u16(bytes, 8U, 97U);
        write_le_u16(bytes, 10U, 17U);
        write_field_descriptor(bytes, 32U, "123BADNAME", 'C', 1U, 8U);
        write_field_descriptor(bytes, 64U, "123BADNAME", 'C', 9U, 8U);
        bytes[96U] = 0x0DU;
        bytes[97U] = 0x20U;
        bytes[128U] = 0x1AU;
        std::ofstream output(field_names_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto field_names_result = copperfin::vfp::inspect_asset(field_names_path.string());
    expect(field_names_result.ok, "inspect_asset should still succeed for DBFs with invalid field names");
    expect(
        has_validation_issue(field_names_result, "dbf.field_name_duplicate", "field_names.dbf"),
        "inspect_asset should report duplicate DBF field names");
    expect(
        has_validation_issue(field_names_result, "dbf.field_name_invalid", "field_names.dbf"),
        "inspect_asset should report invalid DBF field names");

    fs::remove_all(temp_dir, ignored);
}

#include "test_vfp_assets_database_json_contracts.inl"
void test_read_memo_block_raw_returns_correct_bytes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_memo_raw_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "rawtest.dbf";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "ID",   .type = 'N', .offset = 1U, .length = 5U, .decimal_count = 0U},
        {.name = "NOTES",.type = 'M', .offset = 6U, .length = 4U, .decimal_count = 0U}
    };

    // Write a record with a known NOTES value (ASCII — raw bytes preserved)
    const std::vector<std::vector<std::string>> records{{"1", ""}};
    const auto create_result = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, records);
    expect(create_result.ok, "read_memo_block_raw: fixture DBF should be created");

    const std::string memo_content = "Hello memo block";
    const auto write_result = copperfin::vfp::replace_record_field_value(
        dbf_path.string(), 0U, "NOTES", memo_content);
    expect(write_result.ok, "read_memo_block_raw: memo write should succeed");

    // The memo sidecar for .dbf is .fpt (inferred by dbf_table)
    const fs::path fpt_path = fs::path(dbf_path).replace_extension(".fpt");
    if (fs::exists(fpt_path)) {
        const std::vector<std::uint8_t> raw =
            copperfin::vfp::read_memo_block_raw(fpt_path.string(), 1U);
        expect(!raw.empty(), "read_memo_block_raw should return non-empty bytes for block 1");
        if (!raw.empty()) {
            const std::string as_string(raw.begin(), raw.end());
            expect(as_string == memo_content,
                   "read_memo_block_raw should return the exact bytes written to the memo block");
        }
    }

    fs::remove_all(temp_dir, ignored);
}

}  // namespace

int main() {
    test_parse_dbf_header();
    test_last_update_iso8601_applies_fixed_century_rollover_for_real_foxbase_byte();
    test_parse_dbf_header_rejects_short_input();
    test_dbf_cdx_header_errors_resolve_through_localization_catalog();
    test_vfp_header_and_index_default_catalog_refresh();
    test_asset_family_detection();
    test_asset_inspector_errors_resolve_through_localization_catalog();
#if !defined(_WIN32)
    test_inspect_asset_inaccessible_path_returns_structured_failure();
    test_export_database_as_json_inaccessible_path_returns_structured_failure();
#endif
    test_parse_index_probe_for_cdx();
    test_parse_cdx_header_root_offset_beyond_16_bits();
    test_parse_index_probe_for_dcx();
    test_parse_index_probe_for_cdx_prefers_tag_page_local_expressions();
    test_parse_index_probe_for_cdx_binds_descriptive_tag_names_from_tag_page_hints();
    test_parse_index_probe_for_cdx_preserves_plain_field_expression_tags();
    test_parse_index_probe_for_idx();
    test_parse_index_probe_for_ndx();
    test_parse_index_probe_for_ndx_surfaces_character_domain_without_named_collation();
    test_parse_index_probe_for_mdx();
    test_parse_index_probe_for_mdx_rejects_implausible_header();
    test_parse_index_probe_for_ntx();
    test_parse_index_probe_for_ntx_rejects_truncated_header();
    test_parse_index_probe_for_ntx_rejects_partial_trailing_page();
    test_parse_index_probe_for_ntx_rejects_inconsistent_group_length();
    test_index_probe_errors_resolve_through_localization_catalog();
    test_inspect_asset_collects_companion_indexes();
    test_inspect_asset_uses_admitted_index_bytes();
    test_inspect_asset_discovers_virtual_casefolded_index_bytes();
    test_inspect_database_container_collects_dcx_companion();
    test_parse_access_container_header_for_jet3_mdb();
    test_parse_access_container_header_for_jet4_mdb();
    test_parse_access_container_header_for_ace_accdb();
    test_parse_access_container_header_rejects_missing_signature();
    test_parse_access_container_header_rejects_truncated_file();
    test_parse_access_table_definition_page_decodes_jet3_columns();
    test_parse_access_table_definition_page_decodes_jet4_columns();
    test_parse_access_table_definition_page_rejects_wrong_page_size();
    test_parse_access_table_definition_page_rejects_non_tdef_page();
    test_parse_access_table_definition_page_rejects_multi_page_tdef();
    test_parse_access_table_definition_page_rejects_structure_out_of_bounds();
    test_scan_access_container_schema_discovers_tables_and_skips_continuations();
    test_scan_access_container_schema_rejects_non_access_file();
    test_parse_access_table_definition_page_rejects_oversized_index_count();
    test_scan_access_container_schema_rejects_truncated_trailing_page();
    test_scan_access_container_schema_excludes_every_page_in_a_multi_hop_chain();
    test_parse_access_table_definition_page_sanitizes_invalid_jet3_utf8();
    test_parse_access_table_definition_page_rejects_unrecognized_table_type();
    test_scan_access_msysobjects_catalog_decodes_jet3_row();
    test_scan_access_msysobjects_catalog_decodes_jet4_row();
    test_scan_access_msysobjects_catalog_handles_alphabetized_column_storage_order();
    test_scan_access_msysobjects_catalog_skips_deleted_and_lookup_overflow_rows();
    test_scan_access_msysobjects_catalog_fails_closed_on_jet3_row_at_or_above_256_bytes();
    test_scan_access_msysobjects_catalog_fails_closed_on_jet4_compressed_unicode_name();
    test_scan_access_container_schema_attaches_names_from_catalog();
    test_access_container_errors_resolve_through_localization_catalog();
    test_vfp_locale_catalog_parity();
    test_inspect_database_container_collects_casefolded_same_base_companions();
    test_inspect_database_container_extracts_first_pass_catalog_metadata();
    test_inspect_asset_resolves_explicit_unicode_memo_sidecar();
    test_export_database_as_json_resolves_unicode_catalog_table_path();
    test_export_database_as_sql_maps_currency_datetime_and_blank_numeric();
    test_export_database_as_access_sql_maps_currency_datetime_and_dates();
    test_export_database_as_access_sql_escapes_bracket_in_identifier();
    test_export_database_as_access_sql_clamps_decimal_precision();
    test_export_database_as_access_sql_rejects_unsafe_numeric_token();
    test_export_database_as_access_sql_accepts_leading_plus_sign_numeric();
    test_export_database_as_access_sql_rejects_unsafe_date_literal();
    test_database_json_import_plan_admits_exporter_unreadable_table_marker();
    test_parse_real_vfp_cdx_when_available();
    test_parse_additional_real_vfp_cdx_samples_when_available();
    test_parse_real_vfp_dcx_samples_when_available();
    test_parse_real_dbase_ndx_when_available();
    test_inspect_asset_reports_dbf_storage_validation_findings();
    test_inspect_asset_reports_missing_companions_and_unparseable_indexes();
    test_inspect_asset_reports_malformed_memo_sidecar_findings();
    test_inspect_asset_reports_dbf_descriptor_validation_findings();
    test_export_database_as_json_produces_catalog_json();
    test_export_database_as_json_errors_leave_json_empty();
    test_export_database_as_json_decodes_properties_blob();
    test_export_database_as_json_prefers_catalog_name_and_casefolded_assets();
    test_build_database_json_import_plan_validates_without_mutation();
    test_materialize_database_json_import_plan_fails_closed_and_round_trips();
    test_build_database_sql_import_plan_validates_without_mutation();
    test_export_database_as_sql_round_trips_through_import();
    test_read_memo_block_raw_returns_correct_bytes();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
