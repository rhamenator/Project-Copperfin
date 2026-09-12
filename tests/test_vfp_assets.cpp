// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/platform/path.h"
#include "copperfin/vfp/access_container.h"
#include "copperfin/vfp/access_long_value.h"
#include "copperfin/vfp/access_msysobjects.h"
#include "copperfin/vfp/access_saveastext_design.h"
#include "copperfin/vfp/access_saved_queries.h"
#include "copperfin/vfp/access_table_definition.h"
#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/cdx_header.h"
#include "copperfin/vfp/cdx_writer.h"
#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/index_probe.h"
#include "test_environment_support.h"
#include "test_vfp_assets_support.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
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

// #5549: an LVAL page has the same header/row-directory shape as a
// regular data page (make_synthetic_data_page()), except the literal
// ASCII bytes "LVAL" replace the tdef_pg page pointer at offset 4 -- see
// read_access_long_value_column()'s own doc comment for the real-fixture
// evidence this is grounded in.
std::vector<std::uint8_t> make_synthetic_lval_page(
    std::size_t page_size,
    bool is_jet3,
    const std::vector<SyntheticDataRowSlot>& rows) {
    std::vector<std::uint8_t> page = make_synthetic_data_page(page_size, 0U, is_jet3, rows);
    page[4] = 'L';
    page[5] = 'V';
    page[6] = 'A';
    page[7] = 'L';
    return page;
}

// Writes a page_count * page_size zero-filled file with specific pages
// overridden by `pages_by_index` -- read_access_long_value_column() does
// not itself validate a container header (its caller already knows the
// generation), so this deliberately omits one; only page geometry matters.
std::filesystem::path write_synthetic_page_file(
    const std::filesystem::path& temp_dir,
    const std::string& filename,
    std::size_t page_size,
    std::size_t page_count,
    const std::map<std::size_t, std::vector<std::uint8_t>>& pages_by_index) {
    std::vector<std::uint8_t> file_bytes(page_size * page_count, 0U);
    for (const auto& [index, page_bytes] : pages_by_index) {
        std::copy(
            page_bytes.begin(), page_bytes.end(),
            file_bytes.begin() + static_cast<std::ptrdiff_t>(page_size * index));
    }
    const std::filesystem::path path = temp_dir / filename;
    expect(write_binary_file(path, file_bytes), "synthetic LVAL page fixture should be writable");
    return path;
}

std::vector<std::uint8_t> make_long_value_descriptor_bytes(
    std::uint32_t declared_length, std::uint8_t bitmask, std::uint32_t lval_dp) {
    std::vector<std::uint8_t> bytes(12U, 0U);
    bytes[0] = static_cast<std::uint8_t>(declared_length & 0xFFU);
    bytes[1] = static_cast<std::uint8_t>((declared_length >> 8U) & 0xFFU);
    bytes[2] = static_cast<std::uint8_t>((declared_length >> 16U) & 0xFFU);
    bytes[3] = bitmask;
    write_le_u32(bytes, 4U, lval_dp);
    return bytes;
}

// #5477: parse_access_saveastext_design() parses Application.SaveAsText's
// own nested Begin/End text grammar for a form or report. These are
// hand-built SYNTHETIC fixtures, not real Access output -- the grammar
// itself (property syntax, NotDefault marker, blob properties,
// multi-line string continuation with backslash escaping, the
// CodeBehindForm marker) was derived and verified against real Access
// 365 automation output this slice's own investigation produced (see
// docs/78-access-forms-reports-vba-storage-reconnaissance.md and
// docs/79-access-saveastext-design-format-notes.md), matching this
// project's established practice of never committing a real Access-
// derived fixture or its content (#5549/#5551's own precedent).
void test_parse_access_saveastext_design_parses_form_control_hierarchy() {
    const std::string text =
        "Version =19\r\n"
        "VersionRequired =19\r\n"
        "Checksum =12345\r\n"
        "Begin Form\r\n"
        "    Width =7740\r\n"
        "    AllowFilters = NotDefault\r\n"
        "    Caption =\"Order Entry\"\r\n"
        "    GUID = Begin\r\n"
        "        0x47766dbfaadd694db98d5c13cfb68711\r\n"
        "    End\r\n"
        "    Begin\r\n"
        "        Begin Label\r\n"
        "            Name =\"Label1\"\r\n"
        "            Caption =\"Hello\"\r\n"
        "        End\r\n"
        "        Begin CommandButton\r\n"
        "            Name =\"Option1\"\r\n"
        "            OnClick =\"=HandleButtonClick(1)\"\r\n"
        "            Begin\r\n"
        "                Begin Label\r\n"
        "                    Name =\"OptionLabel1\"\r\n"
        "                End\r\n"
        "            End\r\n"
        "        End\r\n"
        "    End\r\n"
        "End\r\n"
        "CodeBehindForm\r\n"
        "Attribute VB_GlobalNameSpace = False\r\n"
        "Option Compare Database\r\n"
        "\r\n"
        "Private Sub Form_Open(Cancel As Integer)\r\n"
        "End Sub\r\n";

    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(result.ok, "well-formed synthetic form design text should parse: " + result.error);
    expect(result.version == 19, "should decode the Version header field");
    expect(result.version_required == 19, "should decode the VersionRequired header field");
    expect(result.checksum == 12345, "should decode the Checksum header field");
    expect(result.root.control_type == "Form", "root control type should be Form");
    expect(result.root.children.size() == 2U, "root Form should have two direct children (Label, CommandButton)");

    const auto* width = result.root.find_property("Width");
    expect(width != nullptr && width->value == "7740" && !width->is_string, "Width should be a plain numeric property");

    const auto* allow_filters = result.root.find_property("AllowFilters");
    expect(allow_filters != nullptr && allow_filters->is_not_default, "AllowFilters should be recognized as the NotDefault marker");

    const auto* caption = result.root.find_property("Caption");
    expect(caption != nullptr && caption->is_string && caption->value == "Order Entry", "Caption should decode as the string \"Order Entry\"");

    const auto* guid = result.root.find_property("GUID");
    expect(guid != nullptr && guid->is_blob && guid->blob_lines.size() == 1U &&
               guid->blob_lines.front() == "        0x47766dbfaadd694db98d5c13cfb68711",
           "GUID should be captured as an opaque one-line blob property, preserving its own original indentation verbatim");

    if (result.root.children.size() == 2U) {
        expect(result.root.children[0].control_type == "Label", "first child should be the Label control");
        expect(result.root.children[1].control_type == "CommandButton", "second child should be the CommandButton control");
        expect(result.root.children[1].children.size() == 1U && result.root.children[1].children[0].control_type == "Label",
               "the CommandButton's own nested anonymous Begin/End block should expose its child Label directly, not as a synthetic wrapper node");
    }

    expect(
        result.code_behind.find("Private Sub Form_Open(Cancel As Integer)") != std::string::npos,
        "code_behind should capture the form's own VBA source verbatim after the CodeBehindForm marker");
    expect(
        result.code_behind.find("Attribute VB_GlobalNameSpace = False") != std::string::npos,
        "code_behind should include the class-module Attribute lines verbatim");
}

void test_parse_access_saveastext_design_accepts_report_root_type() {
    const std::string text =
        "Version =19\r\n"
        "VersionRequired =19\r\n"
        "Checksum =-999\r\n"
        "Begin Report\r\n"
        "    Width =9360\r\n"
        "End\r\n";

    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(result.ok, "a Begin Report root block should be accepted: " + result.error);
    expect(result.root.control_type == "Report", "root control type should be Report");
    expect(result.checksum == -999, "a negative Checksum value should decode correctly");
    expect(result.code_behind.empty(), "a design with no CodeBehindForm marker should report an empty code_behind, not an error");
}

void test_parse_access_saveastext_design_unescapes_multiline_string_continuation() {
    // Mirrors a real fixture's own BaseInfo/ColumnInfo property shape:
    // a long string value wrapped across physical lines with no
    // continuation marker other than each following line itself being
    // nothing but another quoted chunk, plus backslash-escaped quotes
    // and backslashes within the value.
    const std::string text =
        "Version =19\r\n"
        "VersionRequired =19\r\n"
        "Checksum =1\r\n"
        "Begin Form\r\n"
        "    BaseInfo =\"\\\"SELECT * FROM [Pay\"\r\n"
        "        \"ment Methods]\\\";\\\"Primary\"\r\n"
        "        \"Key\\\"\"\r\n"
        "    Picture =\"C:\\\\Program Files\\\\demo.gif\"\r\n"
        "End\r\n";

    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(result.ok, "a multi-line escaped string property should parse: " + result.error);
    const auto* base_info = result.root.find_property("BaseInfo");
    expect(
        base_info != nullptr && base_info->value == "\"SELECT * FROM [Payment Methods]\";\"PrimaryKey\"",
        "multi-line string chunks should concatenate in order and unescape as a whole");
    const auto* picture = result.root.find_property("Picture");
    expect(
        picture != nullptr && picture->value == "C:\\Program Files\\demo.gif",
        "an escaped backslash (\\\\) should unescape to a single literal backslash");
}

void test_parse_access_saveastext_design_rejects_malformed_header() {
    const std::string text = "Version =19\r\nVersionRequired =19\r\nBegin Form\r\nEnd\r\n";  // missing Checksum line
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(!result.ok, "a header missing its Checksum line should fail closed");
}

void test_parse_access_saveastext_design_rejects_unbalanced_block() {
    const std::string text =
        "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Form\r\n    Width =100\r\n";  // no closing End
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(!result.ok, "an unclosed Begin block should fail closed rather than return a partial structure");
}

void test_parse_access_saveastext_design_rejects_unsupported_root_type() {
    const std::string text = "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Macro\r\nEnd\r\n";
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(!result.ok, "a root block type other than Form or Report should fail closed (this slice's own explicit scope)");
}

void test_parse_access_saveastext_design_rejects_malformed_property_line() {
    const std::string text =
        "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Form\r\n    ThisLineHasNoEqualsSign\r\nEnd\r\n";
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(!result.ok, "a line that is neither Begin/End nor a well-formed property assignment should fail closed");
}

void test_parse_access_saveastext_design_rejects_unterminated_string() {
    const std::string text =
        "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Form\r\n    Caption =\"never closed\r\nEnd\r\n";
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(!result.ok, "a quoted string property missing its closing quote should fail closed");
}

void test_parse_access_saveastext_design_rejects_unexpected_trailing_content() {
    const std::string text =
        "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Form\r\nEnd\r\nSomeUnexpectedTrailer\r\n";
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(!result.ok, "trailing content after the root End that isn't a CodeBehindForm marker should fail closed");
}

// PR #5556 review finding (chatgpt-codex-connector/copilot-pull-request-
// reviewer): the original implementation examined only the single line
// immediately following the root End; a blank line there let it silently
// skip past genuinely unexpected trailing content without erroring.
void test_parse_access_saveastext_design_rejects_trailing_content_after_blank_line() {
    const std::string text =
        "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Form\r\nEnd\r\n\r\nSomeUnexpectedTrailer\r\n";
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(
        !result.ok,
        "unexpected trailing content separated from the root End by a blank line should still fail closed, not be silently skipped");
}

// Same review finding, the other half: a blank line before a genuine
// CodeBehindForm marker must not cause the whole code-behind section to
// be silently dropped.
void test_parse_access_saveastext_design_finds_code_behind_after_blank_line() {
    const std::string text =
        "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Form\r\nEnd\r\n\r\nCodeBehindForm\r\nOption Compare Database\r\n";
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(result.ok, "a blank line before CodeBehindForm should not cause a parse failure: " + result.error);
    expect(
        result.code_behind.find("Option Compare Database") != std::string::npos,
        "code_behind must still be captured when a blank line separates the root End from the CodeBehindForm marker");
}

// PR #5556 review finding: code_behind must be byte-exact, including
// its own original line endings -- reconstructing it by rejoining
// already-CR-stripped lines with a fixed '\n' would silently convert a
// real CRLF-terminated export to LF.
void test_parse_access_saveastext_design_preserves_crlf_in_code_behind() {
    const std::string text =
        "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Form\r\nEnd\r\nCodeBehindForm\r\nLine one\r\nLine two\r\n";
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(result.ok, "fixture should parse: " + result.error);
    expect(
        result.code_behind == "Line one\r\nLine two\r\n",
        "code_behind should preserve the source's own original CRLF line endings verbatim, not normalize them to LF");
}

// PR #5556 review finding: a blob property's own raw lines were being
// stored fully whitespace-trimmed, discarding the leading/trailing
// indentation real SaveAsText output has -- the only lossless
// representation available for content this parser deliberately does
// not otherwise decode.
void test_parse_access_saveastext_design_preserves_blob_line_whitespace() {
    const std::string text =
        "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Form\r\n    GUID = Begin\r\n        0xABCDEF  \r\n    End\r\nEnd\r\n";
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(result.ok, "fixture should parse: " + result.error);
    const auto* guid = result.root.find_property("GUID");
    expect(guid != nullptr && guid->is_blob, "GUID should be captured as a blob property");
    expect(
        guid != nullptr && guid->blob_lines.size() == 1U && guid->blob_lines.front() == "        0xABCDEF  ",
        "a blob property's own raw line should keep its original leading/trailing whitespace rather than being fully trimmed");
}

void test_access_saveastext_design_find_property_returns_null_when_absent() {
    const std::string text = "Version =19\r\nVersionRequired =19\r\nChecksum =1\r\nBegin Form\r\n    Width =100\r\nEnd\r\n";
    const auto result = copperfin::vfp::parse_access_saveastext_design(text);
    expect(result.ok, "fixture should parse: " + result.error);
    expect(result.root.find_property("NoSuchProperty") == nullptr, "find_property should return nullptr for an absent property name");
}

void test_parse_access_saveastext_design_from_file_reports_open_failure() {
    const auto result = copperfin::vfp::parse_access_saveastext_design_from_file(
        "/nonexistent/path/that/should/never/exist.txt");
    expect(!result.ok, "parse_access_saveastext_design_from_file should fail closed for a missing file");
}

void test_parse_access_long_value_field_descriptor_decodes_header() {
    const auto bytes = make_long_value_descriptor_bytes(490U, 0x40U, 0x00013D00U);
    const auto descriptor = copperfin::vfp::parse_access_long_value_field_descriptor(bytes);
    expect(descriptor.has_value(), "a well-formed 12-byte descriptor should parse");
    if (descriptor.has_value()) {
        expect(descriptor->declared_length == 490U, "declared_length should match the 3-byte memo_len field");
        expect(descriptor->bitmask == 0x40U, "bitmask should match byte 3");
        expect(descriptor->lval_dp == 0x00013D00U, "lval_dp should match the 4-byte LE field");
    }
}

void test_parse_access_long_value_field_descriptor_rejects_short_input() {
    const std::vector<std::uint8_t> bytes(11U, 0U);
    const auto descriptor = copperfin::vfp::parse_access_long_value_field_descriptor(bytes);
    expect(!descriptor.has_value(), "fewer than 12 bytes should not parse");
}

void test_read_access_long_value_column_returns_inline_value() {
    auto bytes = make_long_value_descriptor_bytes(5U, 0x80U, 0U);
    const std::vector<std::uint8_t> value{'H', 'e', 'l', 'l', 'o'};
    bytes.insert(bytes.end(), value.begin(), value.end());

    const auto result = copperfin::vfp::read_access_long_value_column(
        "", copperfin::vfp::AccessContainerGeneration::jet4, bytes);
    expect(result.ok, "an inline (bitmask 0x80) value should resolve without touching any file: " + result.error);
    expect(result.value == value, "the inline value bytes should be returned unchanged");
}

void test_read_access_long_value_column_rejects_inline_length_mismatch() {
    auto bytes = make_long_value_descriptor_bytes(5U, 0x80U, 0U);
    bytes.insert(bytes.end(), {'H', 'i'});  // only 2 bytes, but declared_length says 5
    const auto result = copperfin::vfp::read_access_long_value_column(
        "", copperfin::vfp::AccessContainerGeneration::jet4, bytes);
    expect(!result.ok, "an inline value whose length disagrees with declared_length should fail closed");
}

void test_read_access_long_value_column_rejects_descriptor_too_short() {
    const std::vector<std::uint8_t> bytes(10U, 0U);
    const auto result = copperfin::vfp::read_access_long_value_column(
        "", copperfin::vfp::AccessContainerGeneration::jet4, bytes);
    expect(!result.ok, "fewer than 12 bytes of column data should fail closed");
}

void test_read_access_long_value_column_rejects_unsupported_bitmask() {
    const auto bytes = make_long_value_descriptor_bytes(5U, 0x20U, 0U);
    const auto result = copperfin::vfp::read_access_long_value_column(
        "", copperfin::vfp::AccessContainerGeneration::jet4, bytes);
    expect(!result.ok, "a bitmask other than 0x80/0x40/0x00 should fail closed");
}

void test_read_access_long_value_column_reads_single_lval_page() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_long_value_single_page_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::vector<std::uint8_t> value(37U, 0xABU);
    const auto lval_page = make_synthetic_lval_page(4096U, false, {{.bytes = value}});
    const auto path = write_synthetic_page_file(temp_dir, "single.bin", 4096U, 10U, {{5U, lval_page}});

    // lval_dp: row_id in the low byte, page number in the upper 3 bytes
    // (real-fixture-verified this session -- see access_long_value.h).
    const auto descriptor_bytes = make_long_value_descriptor_bytes(
        static_cast<std::uint32_t>(value.size()), 0x40U, (5U << 8U) | 0U);

    const auto result = copperfin::vfp::read_access_long_value_column(
        copperfin::platform::path_to_utf8_string(path),
        copperfin::vfp::AccessContainerGeneration::jet4,
        descriptor_bytes);
    expect(result.ok, "a single-LVAL-page (bitmask 0x40) value should resolve: " + result.error);
    expect(result.value == value, "the resolved bytes should match the synthetic LVAL page's row content");

    fs::remove_all(temp_dir, ignored);
}

void test_read_access_long_value_column_reads_chained_lval_pages() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_long_value_chained_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    // Hop 1 (page 6, row 0): 4-byte next pointer to (page 7, row 0), then
    // partial data. Hop 2 (page 7, row 0): 4-byte next pointer of 0 (end
    // of chain), then the remaining partial data. Mirrors the real 15-hop
    // chain this session verified byte-for-byte against a real fixture.
    const std::vector<std::uint8_t> part1(20U, 0x11U);
    const std::vector<std::uint8_t> part2(15U, 0x22U);

    std::vector<std::uint8_t> hop1_row = le_bytes32((7U << 8U) | 0U);
    hop1_row.insert(hop1_row.end(), part1.begin(), part1.end());
    std::vector<std::uint8_t> hop2_row = le_bytes32(0U);
    hop2_row.insert(hop2_row.end(), part2.begin(), part2.end());

    const auto page6 = make_synthetic_lval_page(2048U, true, {{.bytes = hop1_row}});
    const auto page7 = make_synthetic_lval_page(2048U, true, {{.bytes = hop2_row}});
    const auto path = write_synthetic_page_file(
        temp_dir, "chained.bin", 2048U, 10U, {{6U, page6}, {7U, page7}});

    std::vector<std::uint8_t> expected = part1;
    expected.insert(expected.end(), part2.begin(), part2.end());

    const auto descriptor_bytes = make_long_value_descriptor_bytes(
        static_cast<std::uint32_t>(expected.size()), 0x00U, (6U << 8U) | 0U);

    const auto result = copperfin::vfp::read_access_long_value_column(
        copperfin::platform::path_to_utf8_string(path),
        copperfin::vfp::AccessContainerGeneration::jet3,
        descriptor_bytes);
    expect(result.ok, "a chained (bitmask 0x00) value should resolve across multiple LVAL pages: " + result.error);
    expect(result.value == expected, "the reassembled bytes should equal the concatenation of every hop's partial data");

    fs::remove_all(temp_dir, ignored);
}

void test_read_access_long_value_column_rejects_page_without_lval_marker() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_long_value_bad_marker_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    // A regular DATA page (tdef_pg == 2, not the "LVAL" marker) at the
    // target page -- must be rejected, not silently misread as a memo.
    const auto data_page = make_synthetic_data_page(4096U, 2U, false, {{.bytes = {1U, 2U, 3U}}});
    const auto path = write_synthetic_page_file(temp_dir, "wrong_marker.bin", 4096U, 10U, {{5U, data_page}});

    const auto descriptor_bytes = make_long_value_descriptor_bytes(3U, 0x40U, (5U << 8U) | 0U);
    const auto result = copperfin::vfp::read_access_long_value_column(
        copperfin::platform::path_to_utf8_string(path),
        copperfin::vfp::AccessContainerGeneration::jet4,
        descriptor_bytes);
    expect(!result.ok, "a page missing the literal LVAL marker should fail closed rather than misread as a memo page");

    fs::remove_all(temp_dir, ignored);
}

void test_read_access_long_value_column_rejects_row_index_out_of_range() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_long_value_bad_row_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto lval_page = make_synthetic_lval_page(4096U, false, {{.bytes = {1U, 2U, 3U}}});  // 1 row (row_id 0 only)
    const auto path = write_synthetic_page_file(temp_dir, "bad_row.bin", 4096U, 10U, {{5U, lval_page}});

    const auto descriptor_bytes = make_long_value_descriptor_bytes(3U, 0x40U, (5U << 8U) | 3U);  // row_id 3, out of range
    const auto result = copperfin::vfp::read_access_long_value_column(
        copperfin::platform::path_to_utf8_string(path),
        copperfin::vfp::AccessContainerGeneration::jet4,
        descriptor_bytes);
    expect(!result.ok, "a row_id beyond the page's own declared row count should fail closed");

    fs::remove_all(temp_dir, ignored);
}

// #5550 review (Copilot): a deleted/lookup-overflow directory slot must
// fail closed, not be read as real payload.
void test_read_access_long_value_column_rejects_deleted_row_slot() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_long_value_deleted_row_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto lval_page = make_synthetic_lval_page(
        4096U, false, {{.bytes = {1U, 2U, 3U}, .deleted = true}});
    const auto path = write_synthetic_page_file(temp_dir, "deleted_row.bin", 4096U, 10U, {{5U, lval_page}});

    const auto descriptor_bytes = make_long_value_descriptor_bytes(3U, 0x40U, (5U << 8U) | 0U);
    const auto result = copperfin::vfp::read_access_long_value_column(
        copperfin::platform::path_to_utf8_string(path),
        copperfin::vfp::AccessContainerGeneration::jet4,
        descriptor_bytes);
    expect(!result.ok, "a deleted row slot should fail closed rather than being read as payload");

    fs::remove_all(temp_dir, ignored);
}

// #5550 review (Codex P1, Copilot): a chain whose next-pointer loops
// back to an already-visited (page, row) pair must fail closed promptly,
// not re-append the same payload until kMaxChainHops.
void test_read_access_long_value_column_rejects_cyclic_chain() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_long_value_cyclic_chain_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    // Page 6, row 0 points right back at itself.
    std::vector<std::uint8_t> self_referencing_row = le_bytes32((6U << 8U) | 0U);
    self_referencing_row.insert(self_referencing_row.end(), {0xAAU, 0xBBU});
    const auto page6 = make_synthetic_lval_page(2048U, true, {{.bytes = self_referencing_row}});
    const auto path = write_synthetic_page_file(temp_dir, "cyclic.bin", 2048U, 10U, {{6U, page6}});

    const auto descriptor_bytes = make_long_value_descriptor_bytes(1000U, 0x00U, (6U << 8U) | 0U);
    const auto result = copperfin::vfp::read_access_long_value_column(
        copperfin::platform::path_to_utf8_string(path),
        copperfin::vfp::AccessContainerGeneration::jet3,
        descriptor_bytes);
    expect(!result.ok, "a chain that revisits an already-seen pointer should fail closed rather than looping");

    fs::remove_all(temp_dir, ignored);
}

// #5550 review (Codex P1, Copilot): a chain that keeps growing past its
// own declared_length must fail as soon as that becomes evident, not
// only after accumulating far more than the declared length.
void test_read_access_long_value_column_rejects_length_overrun() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_long_value_overrun_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    // A single hop (terminal, next_dp == 0) whose payload is already
    // larger than the descriptor's own declared_length.
    std::vector<std::uint8_t> row = le_bytes32(0U);
    const std::vector<std::uint8_t> payload(50U, 0xCCU);
    row.insert(row.end(), payload.begin(), payload.end());
    const auto page6 = make_synthetic_lval_page(2048U, true, {{.bytes = row}});
    const auto path = write_synthetic_page_file(temp_dir, "overrun.bin", 2048U, 10U, {{6U, page6}});

    const auto descriptor_bytes = make_long_value_descriptor_bytes(10U, 0x00U, (6U << 8U) | 0U);  // declares only 10 bytes
    const auto result = copperfin::vfp::read_access_long_value_column(
        copperfin::platform::path_to_utf8_string(path),
        copperfin::vfp::AccessContainerGeneration::jet3,
        descriptor_bytes);
    expect(!result.ok, "accumulated bytes exceeding declared_length should fail closed");

    fs::remove_all(temp_dir, ignored);
}

// #5550 review (Codex P2): ACE/.accdb (AccessContainerGeneration::later)
// is only extrapolated from Jet4, not real-fixture-verified for this
// capability -- must be explicitly rejected rather than silently parsed
// against the (unverified) Jet4 layout.
void test_read_access_long_value_column_rejects_later_generation() {
    // The inline case (0x80) never touches the file/generation, so use
    // the single-page case (0x40) to actually exercise the check.
    const auto descriptor_bytes = make_long_value_descriptor_bytes(5U, 0x40U, (5U << 8U) | 0U);
    const auto result = copperfin::vfp::read_access_long_value_column(
        "", copperfin::vfp::AccessContainerGeneration::later, descriptor_bytes);
    expect(!result.ok, "AccessContainerGeneration::later should be explicitly rejected, not parsed as Jet4");
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

// #5479: MSysQueries's own column set for saved-query clause rows,
// omitting the "Order" column since scan_access_saved_queries() never
// looks it up by name (matching mdb-queries.c's own scope, which binds
// it but never references it in its reconstruction switch either).
std::vector<SyntheticAccessColumn> make_msysqueries_test_columns() {
    return {
        {.name = "ObjectId", .type = 0x04U, .length = 4U, .fixed = true},
        {.name = "Attribute", .type = 0x02U, .length = 1U, .fixed = true},
        {.name = "Flag", .type = 0x03U, .length = 2U, .fixed = true},
        {.name = "Name1", .type = 0x0AU, .length = 255U, .fixed = false},
        {.name = "Name2", .type = 0x0AU, .length = 255U, .fixed = false},
        {.name = "Expression", .type = 0x0CU, .length = 0U, .fixed = false},
    };
}

// An inline (bitmask 0x80) long-value column's raw bytes: the 12-byte
// field descriptor followed directly by the value -- matches
// read_access_long_value_column()'s own documented inline case (#5549).
std::vector<std::uint8_t> make_inline_memo_bytes(const std::string& text) {
    std::vector<std::uint8_t> bytes(12U, 0U);
    const auto length = static_cast<std::uint32_t>(text.size());
    bytes[0] = static_cast<std::uint8_t>(length & 0xFFU);
    bytes[1] = static_cast<std::uint8_t>((length >> 8U) & 0xFFU);
    bytes[2] = static_cast<std::uint8_t>((length >> 16U) & 0xFFU);
    bytes[3] = 0x80U;
    bytes.insert(bytes.end(), text.begin(), text.end());
    return bytes;
}

void test_scan_access_saved_queries_reconstructs_select_from_where() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_saved_queries_happy_path_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto msysobjects_columns = make_msysobjects_test_columns();
    const std::vector<std::optional<std::vector<std::uint8_t>>> msysobjects_values{
        le_bytes32(100U),
        le_bytes32(0U),
        std::vector<std::uint8_t>{'T', 'e', 's', 't', 'Q', 'u', 'e', 'r', 'y'},
        le_bytes16(5U),  // MDB_QUERY
        std::nullopt,
    };
    const auto msysobjects_row = make_synthetic_msysobjects_row_bytes(msysobjects_columns, msysobjects_values, true);
    // scan_access_container_schema() only attaches a table name via its
    // own MSysObjects catalog join (#5539) -- MSysQueries needs its own
    // catalog row (Id == its own TDEF page number, 4) for
    // scan_access_saved_queries() to find it by name.
    const std::vector<std::optional<std::vector<std::uint8_t>>> msysqueries_catalog_values{
        le_bytes32(4U),
        le_bytes32(0U),
        std::vector<std::uint8_t>{'M', 'S', 'y', 's', 'Q', 'u', 'e', 'r', 'i', 'e', 's'},
        le_bytes16(1U),  // MDB_TABLE
        std::nullopt,
    };
    const auto msysqueries_catalog_row =
        make_synthetic_msysobjects_row_bytes(msysobjects_columns, msysqueries_catalog_values, true);
    const auto msysobjects_data_page = make_synthetic_data_page(
        2048U, 2U, true, {{.bytes = msysobjects_row}, {.bytes = msysqueries_catalog_row}});

    const auto msysqueries_columns = make_msysqueries_test_columns();
    // MSysQueries's own TDEF lands at page 4 (trailing_pages[1], since
    // trailing_pages[0] is the MSysObjects data page at page 3).
    const auto msysqueries_tdef_page = make_synthetic_jet3_tdef_page(msysqueries_columns, 0x4EU, 3U);

    const auto table_row = make_synthetic_msysobjects_row_bytes(
        msysqueries_columns,
        {le_bytes32(100U), std::vector<std::uint8_t>{5U}, le_bytes16(0U),
         std::vector<std::uint8_t>{'M', 'y', 'T', 'a', 'b', 'l', 'e'}, std::nullopt, std::nullopt},
        true);
    const auto column_row = make_synthetic_msysobjects_row_bytes(
        msysqueries_columns,
        {le_bytes32(100U), std::vector<std::uint8_t>{6U}, le_bytes16(0U), std::nullopt, std::nullopt,
         make_inline_memo_bytes("MyTable.MyColumn")},
        true);
    const auto where_row = make_synthetic_msysobjects_row_bytes(
        msysqueries_columns,
        {le_bytes32(100U), std::vector<std::uint8_t>{8U}, le_bytes16(0U), std::nullopt, std::nullopt,
         make_inline_memo_bytes("MyTable.MyColumn > 5")},
        true);
    const auto msysqueries_data_page =
        make_synthetic_data_page(2048U, 4U, true, {{.bytes = table_row}, {.bytes = column_row}, {.bytes = where_row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "queries.mdb", true, msysobjects_columns,
        {msysobjects_data_page, msysqueries_tdef_page, msysqueries_data_page}, 2U);

    const auto result = copperfin::vfp::scan_access_saved_queries(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan_access_saved_queries should succeed for a well-formed synthetic container: " + result.error);
    expect(result.skipped.empty(), "a fully-decodable query should not be reported as skipped");
    expect(result.queries.size() == 1U, "exactly one query should be discovered");
    if (result.queries.size() == 1U) {
        expect(result.queries[0].name == "TestQuery", "the discovered query should be named from its catalog entry");
        expect(result.queries[0].sql == "SELECT MyTable.MyColumn FROM [MyTable] WHERE MyTable.MyColumn > 5",
               "the reconstructed SQL should match the Attribute-based algorithm: got '" + result.queries[0].sql + "'");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_saved_queries_skips_query_with_undecodable_clause_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_saved_queries_mode_switch_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto msysobjects_columns = make_msysobjects_test_columns();
    const std::vector<std::optional<std::vector<std::uint8_t>>> msysobjects_values{
        le_bytes32(200U),
        le_bytes32(0U),
        ucs2le_bytes("BadQuery"),
        le_bytes16(5U),  // MDB_QUERY
        std::nullopt,
    };
    const auto msysobjects_row = make_synthetic_msysobjects_row_bytes(msysobjects_columns, msysobjects_values, false);
    const std::vector<std::optional<std::vector<std::uint8_t>>> msysqueries_catalog_values{
        le_bytes32(4U),
        le_bytes32(0U),
        ucs2le_bytes("MSysQueries"),
        le_bytes16(1U),  // MDB_TABLE
        std::nullopt,
    };
    const auto msysqueries_catalog_row =
        make_synthetic_msysobjects_row_bytes(msysobjects_columns, msysqueries_catalog_values, false);
    const auto msysobjects_data_page = make_synthetic_data_page(
        4096U, 2U, false, {{.bytes = msysobjects_row}, {.bytes = msysqueries_catalog_row}});

    const auto msysqueries_columns = make_msysqueries_test_columns();
    const auto msysqueries_tdef_page = make_synthetic_jet4_tdef_page(msysqueries_columns, 0x4EU, 1U);

    // A Jet4 "compressed unicode" Expression containing an embedded 0x00
    // mode-switch byte -- the case this slice deliberately does not
    // interpret (see access_saved_queries.cpp's own decode_jet4_text()
    // comment). Bytes: 0xFF 0xFE marker, then 'A' (0x41), then the 0x00
    // mode-switch byte, then trailing bytes that are never reached.
    std::vector<std::uint8_t> mode_switch_expression{0xFFU, 0xFEU, 0x41U, 0x00U, 0x42U, 0x00U};
    std::vector<std::uint8_t> expression_descriptor(12U, 0U);
    const auto expr_length = static_cast<std::uint32_t>(mode_switch_expression.size());
    expression_descriptor[0] = static_cast<std::uint8_t>(expr_length & 0xFFU);
    expression_descriptor[1] = static_cast<std::uint8_t>((expr_length >> 8U) & 0xFFU);
    expression_descriptor[3] = 0x80U;
    expression_descriptor.insert(expression_descriptor.end(), mode_switch_expression.begin(), mode_switch_expression.end());

    const auto column_row = make_synthetic_msysobjects_row_bytes(
        msysqueries_columns,
        {le_bytes32(200U), std::vector<std::uint8_t>{6U}, le_bytes16(0U), std::nullopt, std::nullopt,
         expression_descriptor},
        false);
    const auto msysqueries_data_page = make_synthetic_data_page(4096U, 4U, false, {{.bytes = column_row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "badquery.mdb", false, msysobjects_columns,
        {msysobjects_data_page, msysqueries_tdef_page, msysqueries_data_page}, 2U);

    const auto result = copperfin::vfp::scan_access_saved_queries(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "the scan itself should still succeed even though one query is incomplete: " + result.error);
    expect(result.queries.empty(),
           "a query with an undecodable clause row should not be reconstructed with silently-missing pieces");
    expect(result.skipped.size() == 1U && result.skipped[0].name == "BadQuery",
           "the query with the undecodable clause row should be reported as skipped, not silently dropped or partially built");

    fs::remove_all(temp_dir, ignored);
}

// #5551 review (Codex P1): a query sorted by two or more fields must
// have every ORDER BY expression preserved, not just the first.
void test_scan_access_saved_queries_appends_multiple_order_by_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_saved_queries_multi_order_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto msysobjects_columns = make_msysobjects_test_columns();
    const auto msysobjects_row = make_synthetic_msysobjects_row_bytes(
        msysobjects_columns,
        {le_bytes32(100U), le_bytes32(0U), std::vector<std::uint8_t>{'M', 'u', 'l', 't', 'i', 'O', 'r', 'd', 'e', 'r'},
         le_bytes16(5U), std::nullopt},
        true);
    const auto msysqueries_catalog_row = make_synthetic_msysobjects_row_bytes(
        msysobjects_columns,
        {le_bytes32(4U), le_bytes32(0U),
         std::vector<std::uint8_t>{'M', 'S', 'y', 's', 'Q', 'u', 'e', 'r', 'i', 'e', 's'}, le_bytes16(1U),
         std::nullopt},
        true);
    const auto msysobjects_data_page = make_synthetic_data_page(
        2048U, 2U, true, {{.bytes = msysobjects_row}, {.bytes = msysqueries_catalog_row}});

    const auto msysqueries_columns = make_msysqueries_test_columns();
    const auto msysqueries_tdef_page = make_synthetic_jet3_tdef_page(msysqueries_columns, 0x4EU, 3U);

    const auto table_row = make_synthetic_msysobjects_row_bytes(
        msysqueries_columns,
        {le_bytes32(100U), std::vector<std::uint8_t>{5U}, le_bytes16(0U),
         std::vector<std::uint8_t>{'T'}, std::nullopt, std::nullopt},
        true);
    const auto order_a_row = make_synthetic_msysobjects_row_bytes(
        msysqueries_columns,
        {le_bytes32(100U), std::vector<std::uint8_t>{11U}, le_bytes16(0U),
         std::vector<std::uint8_t>{'D'}, std::nullopt, make_inline_memo_bytes("T.A")},
        true);
    const auto order_b_row = make_synthetic_msysobjects_row_bytes(
        msysqueries_columns,
        {le_bytes32(100U), std::vector<std::uint8_t>{11U}, le_bytes16(0U), std::nullopt, std::nullopt,
         make_inline_memo_bytes("T.B")},
        true);
    const auto msysqueries_data_page = make_synthetic_data_page(
        2048U, 4U, true, {{.bytes = table_row}, {.bytes = order_a_row}, {.bytes = order_b_row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "multiorder.mdb", true, msysobjects_columns,
        {msysobjects_data_page, msysqueries_tdef_page, msysqueries_data_page}, 2U);

    const auto result = copperfin::vfp::scan_access_saved_queries(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan should succeed: " + result.error);
    expect(result.queries.size() == 1U, "exactly one query should be discovered");
    if (result.queries.size() == 1U) {
        expect(result.queries[0].sql == "SELECT  FROM [T] ORDER BY T.A DESCENDING,T.B",
               "both ORDER BY expressions should be preserved, comma-separated: got '" + result.queries[0].sql + "'");
    }

    fs::remove_all(temp_dir, ignored);
}

// #5551 review (Codex P1, Copilot): a query with clause rows but no
// table reference (Attribute 5) must not be reconstructed as a
// fabricated SELECT with an empty FROM clause.
void test_scan_access_saved_queries_skips_query_with_no_table_reference() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_saved_queries_no_table_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto msysobjects_columns = make_msysobjects_test_columns();
    const auto msysobjects_row = make_synthetic_msysobjects_row_bytes(
        msysobjects_columns,
        {le_bytes32(100U), le_bytes32(0U),
         std::vector<std::uint8_t>{'N', 'o', 'T', 'a', 'b', 'l', 'e'}, le_bytes16(5U), std::nullopt},
        true);
    const auto msysqueries_catalog_row = make_synthetic_msysobjects_row_bytes(
        msysobjects_columns,
        {le_bytes32(4U), le_bytes32(0U),
         std::vector<std::uint8_t>{'M', 'S', 'y', 's', 'Q', 'u', 'e', 'r', 'i', 'e', 's'}, le_bytes16(1U),
         std::nullopt},
        true);
    const auto msysobjects_data_page = make_synthetic_data_page(
        2048U, 2U, true, {{.bytes = msysobjects_row}, {.bytes = msysqueries_catalog_row}});

    const auto msysqueries_columns = make_msysqueries_test_columns();
    const auto msysqueries_tdef_page = make_synthetic_jet3_tdef_page(msysqueries_columns, 0x4EU, 1U);

    // Only an Attribute==6 (column) row -- no Attribute==5 (table) row at
    // all, mimicking what a non-SELECT query type might look like.
    const auto column_row = make_synthetic_msysobjects_row_bytes(
        msysqueries_columns,
        {le_bytes32(100U), std::vector<std::uint8_t>{6U}, le_bytes16(0U), std::nullopt, std::nullopt,
         make_inline_memo_bytes("SomeExpression")},
        true);
    const auto msysqueries_data_page = make_synthetic_data_page(2048U, 4U, true, {{.bytes = column_row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "notable.mdb", true, msysobjects_columns,
        {msysobjects_data_page, msysqueries_tdef_page, msysqueries_data_page}, 2U);

    const auto result = copperfin::vfp::scan_access_saved_queries(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "scan should still succeed: " + result.error);
    expect(result.queries.empty(), "a query with no table reference should not be reconstructed as a fabricated SELECT");
    expect(result.skipped.size() == 1U && result.skipped[0].name == "NoTable",
           "the query should be reported as skipped for lacking a table reference");

    fs::remove_all(temp_dir, ignored);
}

void test_scan_access_saved_queries_reports_ok_with_no_queries() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_access_saved_queries_none_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto msysobjects_columns = make_msysobjects_test_columns();
    const std::vector<std::optional<std::vector<std::uint8_t>>> msysobjects_values{
        le_bytes32(2U),
        le_bytes32(251658241U),
        std::vector<std::uint8_t>{'M', 'S', 'y', 's', 'O', 'b', 'j', 'e', 'c', 't', 's'},
        le_bytes16(1U),  // MDB_TABLE, not a query
        std::nullopt,
    };
    const auto msysobjects_row = make_synthetic_msysobjects_row_bytes(msysobjects_columns, msysobjects_values, true);
    const auto data_page = make_synthetic_data_page(2048U, 2U, true, {{.bytes = msysobjects_row}});

    const auto container_path = write_synthetic_msysobjects_container(
        temp_dir, "noqueries.mdb", true, msysobjects_columns, {data_page}, 1U);

    const auto result = copperfin::vfp::scan_access_saved_queries(
        copperfin::platform::path_to_utf8_string(container_path));
    expect(result.ok, "a container with no query-type catalog entries should still succeed: " + result.error);
    expect(result.queries.empty(), "no queries should be discovered");
    expect(result.skipped.empty(), "nothing should be reported as skipped when there was nothing to skip");

    fs::remove_all(temp_dir, ignored);
}

// #5534 (index-rebuild half): create_vfp_cdx_single_tag_index_file() writes
// a real-VFP9-verified single-tag CDX (see docs/77 for the evidence
// trail). This codebase's own CDX reader (index_probe.cpp,
// cdx_header.cpp) is explicitly header-probe-only and its tag/key-
// expression discovery is a best-effort heuristic tuned against real
// multi-tag VFP9 output (e.g. it expects a tag's page-offset pointer
// immediately after the directory-leaf page header, and searches a
// window derived from that pointer for expression-shaped text) -- it
// was not built against, and does not reliably round-trip, this
// writer's own single-tag page layout (confirmed independently
// correct against real VFP9 itself, per docs/77). So this test
// verifies the tag name via that reader (which does work, since names
// are read from a fixed directory-leaf slot the reader already
// understands) but decodes the key-expression page and the leaf
// entries' own front-compression bytes directly, against this
// writer's own documented format, rather than through that reader.
struct DecodedCdxLeafEntry {
    std::uint8_t record_number = 0;
    std::string key;
};

// Mirrors cdx_writer.cpp's own build_leaf_page() front-compression
// algorithm in reverse (see that function's comments and docs/77's
// "Front-compression algorithm" section): the entry array at
// [24: 24+2*entry_count) holds ascending-order (record_number,
// control_byte) pairs, and each entry's own trailing/differing suffix
// text is packed back-to-back working backward from the page end, in
// the SAME (ascending) entry order -- so entry 0's own trailing text
// occupies the last bytes of the page, entry 1's sits just before
// that, and so on.
std::vector<DecodedCdxLeafEntry> decode_cdx_leaf_page_for_test(
    const std::vector<std::uint8_t>& file_bytes, std::size_t leaf_page_offset) {
    std::vector<DecodedCdxLeafEntry> decoded;
    if (leaf_page_offset + 512U > file_bytes.size()) {
        return decoded;
    }
    const std::uint16_t entry_count = static_cast<std::uint16_t>(
        file_bytes[leaf_page_offset + 2U] | (static_cast<std::uint16_t>(file_bytes[leaf_page_offset + 3U]) << 8U));

    std::size_t text_cursor = leaf_page_offset + 512U;
    std::string previous_key;
    for (std::uint16_t index = 0; index < entry_count; ++index) {
        const std::size_t entry_offset = leaf_page_offset + 24U + static_cast<std::size_t>(index) * 2U;
        const std::uint8_t record_number = file_bytes[entry_offset];
        const std::uint8_t control_byte = file_bytes[entry_offset + 1U];
        const std::size_t full_length = static_cast<std::size_t>((control_byte >> 4U) & 0x0FU);
        const std::size_t shared_length = static_cast<std::size_t>(control_byte & 0x0FU);
        const std::size_t trailing_length = full_length - shared_length;
        text_cursor -= trailing_length;
        const std::string trailing(
            file_bytes.begin() + static_cast<std::ptrdiff_t>(text_cursor),
            file_bytes.begin() + static_cast<std::ptrdiff_t>(text_cursor + trailing_length));
        const std::string key = previous_key.substr(0, shared_length) + trailing;
        decoded.push_back({.record_number = record_number, .key = key});
        previous_key = key;
    }
    return decoded;
}

void test_create_vfp_cdx_single_tag_index_file_writes_readable_tag_and_sets_production_index_flag() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_happy_path_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    const fs::path cdx_path = temp_dir / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}};
    const std::vector<std::vector<std::string>> records{{"BANANA"}, {"APPLE"}, {"CHERRY"}};

    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, records);
    expect(dbf_create.ok, "CDX writer happy-path test: DBF fixture should be created: " + dbf_create.error);

    const std::vector<copperfin::vfp::CdxIndexEntry> entries{
        {.record_number = 1U, .key_field_value = "BANANA"},
        {.record_number = 2U, .key_field_value = "APPLE"},
        {.record_number = 3U, .key_field_value = "CHERRY"}};

    const auto write_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "fruitname", "NAME", 10U, entries);
    expect(write_result.ok, "create_vfp_cdx_single_tag_index_file should succeed for a well-formed single tag: " + write_result.error);

    expect(fs::exists(cdx_path), "create_vfp_cdx_single_tag_index_file should create the destination file");
    expect(fs::file_size(cdx_path) == 6U * 512U, "a single-leaf-page CDX should be exactly six 512-byte pages");

    const auto probe_result = copperfin::vfp::parse_index_probe_from_file(cdx_path.string());
    expect(probe_result.ok, "the written CDX should parse as a valid CDX-family index: " + probe_result.error);
    expect(probe_result.probe.kind == copperfin::vfp::IndexKind::cdx, "the written CDX should be typed as CDX");
    expect(!probe_result.probe.tags.empty(), "the written CDX should expose at least one tag");
    if (!probe_result.probe.tags.empty()) {
        expect(probe_result.probe.tags.front().name_hint == "FRUITNAME", "the written CDX should expose the tag name, uppercased");
    }

    std::vector<std::uint8_t> raw_cdx_bytes(static_cast<std::size_t>(fs::file_size(cdx_path)), 0U);
    {
        std::ifstream input(cdx_path, std::ios::binary);
        input.read(reinterpret_cast<char*>(raw_cdx_bytes.data()), static_cast<std::streamsize>(raw_cdx_bytes.size()));
    }

    // Key-expression page (page 4, byte offset 2048) -- see docs/77.
    constexpr std::size_t key_expression_page_offset = 4U * 512U;
    const std::string stored_key_expression(
        raw_cdx_bytes.begin() + static_cast<std::ptrdiff_t>(key_expression_page_offset),
        raw_cdx_bytes.begin() + static_cast<std::ptrdiff_t>(key_expression_page_offset + 4U));
    expect(stored_key_expression == "NAME", "the key-expression page should hold the key expression text verbatim at byte 0");
    expect(
        raw_cdx_bytes[key_expression_page_offset + 4U] == 0U,
        "the key-expression page should be NUL-padded immediately after the expression text");

    // Leaf data page (page 5, byte offset 2560) -- decode the
    // front-compressed entries directly against this writer's own
    // documented algorithm and confirm they reconstruct the three
    // entries, in ascending key order, with their original record
    // numbers.
    constexpr std::size_t leaf_page_offset = 5U * 512U;
    const std::vector<DecodedCdxLeafEntry> decoded_entries =
        decode_cdx_leaf_page_for_test(raw_cdx_bytes, leaf_page_offset);
    expect(decoded_entries.size() == 3U, "the leaf page should decode exactly three entries");
    if (decoded_entries.size() == 3U) {
        expect(decoded_entries[0].key == "APPLE" && decoded_entries[0].record_number == 2U, "the leaf page's first entry should be APPLE/2 in ascending order");
        expect(decoded_entries[1].key == "BANANA" && decoded_entries[1].record_number == 1U, "the leaf page's second entry should be BANANA/1 in ascending order");
        expect(decoded_entries[2].key == "CHERRY" && decoded_entries[2].record_number == 3U, "the leaf page's third entry should be CHERRY/3 in ascending order");
    }

    const auto dbf_header_result = copperfin::vfp::parse_dbf_header_from_file(dbf_path.string());
    expect(dbf_header_result.ok, "the DBF fixture should still parse after indexing: " + dbf_header_result.error);
    expect(
        dbf_header_result.header.has_production_index(),
        "create_vfp_cdx_single_tag_index_file should set the DBF's has_production_index flag");

    fs::remove_all(temp_dir, ignored);
}

void test_create_vfp_cdx_single_tag_index_file_rejects_empty_tag_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_empty_tag_name_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    const fs::path cdx_path = temp_dir / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}};
    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, {{"BANANA"}});
    expect(dbf_create.ok, "CDX writer empty-tag-name test: DBF fixture should be created: " + dbf_create.error);

    const std::vector<copperfin::vfp::CdxIndexEntry> entries{{.record_number = 1U, .key_field_value = "BANANA"}};
    const auto write_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "", "NAME", 10U, entries);
    expect(!write_result.ok, "create_vfp_cdx_single_tag_index_file should reject an empty tag name");
    expect(!write_result.error.empty(), "an empty-tag-name rejection should carry a localized error message");
    expect(!fs::exists(cdx_path), "create_vfp_cdx_single_tag_index_file should not create a file when the tag name is rejected");

    fs::remove_all(temp_dir, ignored);
}

void test_create_vfp_cdx_single_tag_index_file_rejects_key_exceeding_declared_length() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_key_too_long_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    const fs::path cdx_path = temp_dir / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 20U, .decimal_count = 0U}};
    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, {{"WATERMELON"}});
    expect(dbf_create.ok, "CDX writer key-too-long test: DBF fixture should be created: " + dbf_create.error);

    // Declares a key length (5) shorter than the entry's own trimmed
    // value (10 characters) -- should fail closed rather than silently
    // truncate the key.
    const std::vector<copperfin::vfp::CdxIndexEntry> entries{{.record_number = 1U, .key_field_value = "WATERMELON"}};
    const auto write_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "fruitname", "NAME", 5U, entries);
    expect(!write_result.ok, "create_vfp_cdx_single_tag_index_file should reject a key longer than the declared key length");
    expect(!fs::exists(cdx_path), "create_vfp_cdx_single_tag_index_file should not create a file when a key is rejected");

    fs::remove_all(temp_dir, ignored);
}

void test_create_vfp_cdx_single_tag_index_file_rejects_key_longer_than_compression_encoding_limit() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_key_length_unsupported_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    const fs::path cdx_path = temp_dir / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 20U, .decimal_count = 0U}};
    // 16 significant characters -- one past the front-compression
    // control byte's 4-bit length nibble (max 15), even though the
    // declared key_length (20) is large enough to hold it.
    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, {{"SIXTEENCHARACTRS"}});
    expect(dbf_create.ok, "CDX writer key-length-unsupported test: DBF fixture should be created: " + dbf_create.error);

    const std::vector<copperfin::vfp::CdxIndexEntry> entries{
        {.record_number = 1U, .key_field_value = "SIXTEENCHARACTRS"}};
    const auto write_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "fruitname", "NAME", 20U, entries);
    expect(
        !write_result.ok,
        "create_vfp_cdx_single_tag_index_file should reject a key longer than the front-compression control byte's 4-bit length limit");
    expect(!fs::exists(cdx_path), "create_vfp_cdx_single_tag_index_file should not create a file when a key is too long to encode");

    fs::remove_all(temp_dir, ignored);
}

// PR #5552 review finding (chatgpt-codex-connector/copilot-pull-request-
// reviewer): CdxIndexEntry::record_number is deliberately a wider type
// than the single byte the leaf entry format can hold, precisely so a
// caller can't silently narrow (e.g. record 256 -> 0) before this
// function ever sees the out-of-range value; this must fail closed.
void test_create_vfp_cdx_single_tag_index_file_rejects_record_number_out_of_range() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_record_number_out_of_range_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    const fs::path cdx_path = temp_dir / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}};
    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, {{"BANANA"}});
    expect(dbf_create.ok, "CDX writer record-number-out-of-range test: DBF fixture should be created: " + dbf_create.error);

    const std::vector<copperfin::vfp::CdxIndexEntry> entries{{.record_number = 256U, .key_field_value = "BANANA"}};
    const auto write_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "fruitname", "NAME", 10U, entries);
    expect(!write_result.ok, "create_vfp_cdx_single_tag_index_file should reject a record number above 255");
    expect(!fs::exists(cdx_path), "create_vfp_cdx_single_tag_index_file should not create a file when a record number is out of range");

    const auto dbf_header_result = copperfin::vfp::parse_dbf_header_from_file(dbf_path.string());
    expect(dbf_header_result.ok, "the DBF fixture should still parse after the rejected write: " + dbf_header_result.error);
    expect(
        !dbf_header_result.header.has_production_index(),
        "a rejected write must not set the DBF's has_production_index flag");

    fs::remove_all(temp_dir, ignored);
}

// PR #5552 review finding (copilot-pull-request-reviewer): key_length
// was accepted without validation even though 0 or a value larger than
// one page makes the resulting file unparseable by this codebase's own
// CDX reader (cdx_header.cpp's collect_directory_leaf_tags()).
void test_create_vfp_cdx_single_tag_index_file_rejects_invalid_key_length() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_invalid_key_length_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    const fs::path cdx_path = temp_dir / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}};
    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, {{"BANANA"}});
    expect(dbf_create.ok, "CDX writer invalid-key-length test: DBF fixture should be created: " + dbf_create.error);

    const std::vector<copperfin::vfp::CdxIndexEntry> entries{{.record_number = 1U, .key_field_value = "BANANA"}};

    const auto zero_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "fruitname", "NAME", 0U, entries);
    expect(!zero_result.ok, "create_vfp_cdx_single_tag_index_file should reject a zero key length");

    const auto oversized_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "fruitname", "NAME", 513U, entries);
    expect(!oversized_result.ok, "create_vfp_cdx_single_tag_index_file should reject a key length larger than one page");

    expect(!fs::exists(cdx_path), "create_vfp_cdx_single_tag_index_file should not create a file when the key length is invalid");

    fs::remove_all(temp_dir, ignored);
}

// PR #5552 review finding (chatgpt-codex-connector/copilot-pull-request-
// reviewer): a key expression of 512+ bytes was silently truncated (and
// an exactly-512-byte expression left with no NUL terminator) rather
// than rejected.
void test_create_vfp_cdx_single_tag_index_file_rejects_oversized_key_expression() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_oversized_key_expression_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    const fs::path cdx_path = temp_dir / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}};
    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, {{"BANANA"}});
    expect(dbf_create.ok, "CDX writer oversized-key-expression test: DBF fixture should be created: " + dbf_create.error);

    const std::vector<copperfin::vfp::CdxIndexEntry> entries{{.record_number = 1U, .key_field_value = "BANANA"}};
    const std::string oversized_expression(512U, 'N');  // exactly one page, no room for a NUL terminator.
    const auto write_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "fruitname", oversized_expression, 10U, entries);
    expect(!write_result.ok, "create_vfp_cdx_single_tag_index_file should reject a key expression that fills the whole page");
    expect(!fs::exists(cdx_path), "create_vfp_cdx_single_tag_index_file should not create a file when the key expression is too long");

    fs::remove_all(temp_dir, ignored);
}

// PR #5552 review finding (chatgpt-codex-connector): tag names of
// 481-512 bytes previously passed the old (too-loose) length check but
// then overwrote the tag-table page's own required header fields at
// bytes [0:32) once right-aligned to the page end.
void test_create_vfp_cdx_single_tag_index_file_rejects_tag_name_that_would_overwrite_header() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_tag_name_overwrite_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    const fs::path cdx_path = temp_dir / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}};
    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, {{"BANANA"}});
    expect(dbf_create.ok, "CDX writer tag-name-overwrite test: DBF fixture should be created: " + dbf_create.error);

    const std::vector<copperfin::vfp::CdxIndexEntry> entries{{.record_number = 1U, .key_field_value = "BANANA"}};
    const std::string long_tag_name(481U, 'T');  // one byte past the 480-byte safe limit.
    const auto write_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), long_tag_name, "NAME", 10U, entries);
    expect(!write_result.ok, "create_vfp_cdx_single_tag_index_file should reject a tag name that would overwrite the tag-table page's header fields");
    expect(!fs::exists(cdx_path), "create_vfp_cdx_single_tag_index_file should not create a file when the tag name is rejected");

    fs::remove_all(temp_dir, ignored);
}

// PR #5552 review finding (chatgpt-codex-connector/copilot-pull-request-
// reviewer, reported twice each): the DBF has_production_index flag was
// being set BEFORE the CDX file was known to be durably written, so a
// write failure left the table falsely claiming a structural index it
// didn't have. create_vfp_cdx_single_tag_index_file() now durably
// writes the CDX (via a temp-file-then-rename stage) FIRST and only
// then sets the flag; this test forces the CDX write itself to fail
// (an unwritable destination directory) and confirms the flag stays
// unset.
void test_create_vfp_cdx_single_tag_index_file_does_not_set_production_index_flag_when_cdx_write_fails() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_write_failure_ordering_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    // A CDX path inside a directory that does not exist: opening the
    // temp file for writing must fail deterministically, independent of
    // platform-specific permission behavior.
    const fs::path cdx_path = temp_dir / "no-such-subdirectory" / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}};
    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, {{"BANANA"}});
    expect(dbf_create.ok, "CDX writer write-failure-ordering test: DBF fixture should be created: " + dbf_create.error);

    const std::vector<copperfin::vfp::CdxIndexEntry> entries{{.record_number = 1U, .key_field_value = "BANANA"}};
    const auto write_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "fruitname", "NAME", 10U, entries);
    expect(!write_result.ok, "create_vfp_cdx_single_tag_index_file should fail when the CDX destination cannot be opened");

    const auto dbf_header_result = copperfin::vfp::parse_dbf_header_from_file(dbf_path.string());
    expect(dbf_header_result.ok, "the DBF fixture should still parse after the failed write: " + dbf_header_result.error);
    expect(
        !dbf_header_result.header.has_production_index(),
        "a failed CDX write must not leave the DBF's has_production_index flag set");

    fs::remove_all(temp_dir, ignored);
}

void test_create_vfp_cdx_single_tag_index_file_rejects_leaf_page_overflow() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_cdx_writer_leaf_overflow_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbf_path = temp_dir / "fruit.dbf";
    const fs::path cdx_path = temp_dir / "fruit.cdx";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 15U, .decimal_count = 0U}};

    // 15-byte keys (the writer's own per-key compression-encoding limit,
    // see the KeyLengthUnsupported test below) whose first three
    // characters are a base-26 encoding of the entry's own index -- at
    // most two of 40 entries ever share a first character, and those
    // two always differ at the second character, so no two entries ever
    // share more than a 1-byte prefix and front-compression cannot
    // meaningfully help. Each entry then costs roughly
    // 2 (record_number, control_byte) + ~14 (trailing text) = 16 bytes,
    // well over the 512-byte leaf page's ~488 usable bytes once 40 such
    // entries are combined -- deliberately overflowing the
    // single-leaf-page scope this writer covers (see docs/77's "Known
    // gaps").
    std::vector<std::vector<std::string>> records;
    std::vector<copperfin::vfp::CdxIndexEntry> entries;
    for (int index = 0; index < 40; ++index) {
        std::string key(15U, 'Z');
        int remainder = index;
        key[0] = static_cast<char>('A' + (remainder % 26));
        remainder /= 26;
        key[1] = static_cast<char>('A' + (remainder % 26));
        records.push_back({key});
        entries.push_back({.record_number = static_cast<std::uint8_t>(index + 1), .key_field_value = key});
    }

    const auto dbf_create = copperfin::vfp::create_dbf_table_file(dbf_path.string(), fields, records);
    expect(dbf_create.ok, "CDX writer leaf-overflow test: DBF fixture should be created: " + dbf_create.error);

    const auto write_result = copperfin::vfp::create_vfp_cdx_single_tag_index_file(
        cdx_path.string(), dbf_path.string(), "fruitname", "NAME", 15U, entries);
    expect(!write_result.ok, "create_vfp_cdx_single_tag_index_file should fail closed when entries overflow a single leaf page");
    expect(!fs::exists(cdx_path), "create_vfp_cdx_single_tag_index_file should not create a file when the leaf page overflows");

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
        "Vfp.AccessDesign.Error.MalformedHeader",
        "Vfp.AccessDesign.Error.MalformedPropertyLine",
        "Vfp.AccessDesign.Error.MissingRootBlock",
        "Vfp.AccessDesign.Error.OpenFileFailed",
        "Vfp.AccessDesign.Error.ReadFileFailed",
        "Vfp.AccessDesign.Error.UnbalancedBlock",
        "Vfp.AccessDesign.Error.UnexpectedTrailingContent",
        "Vfp.AccessDesign.Error.UnsupportedRootType",
        "Vfp.AccessDesign.Error.UnterminatedString",
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
        "Vfp.AssetInspector.Error.DbcTableEscapesDirectory",
        "Vfp.AssetInspector.Error.DbcTableNameUnsafe",
        "Vfp.AssetInspector.Error.PathMissing",
        "Vfp.AssetInspector.Error.ReadFailed",
        "Vfp.AssetInspector.Validation.DbcCatalogEmpty",
        "Vfp.AssetInspector.Validation.DbcCatalogParseFailed",
        "Vfp.AssetInspector.Validation.DbfDescriptorSpanMisaligned",
        "Vfp.AssetInspector.Validation.DbfDescriptorTerminatorMissing",
        "Vfp.AssetInspector.Validation.DbfDescriptorTerminatorNoRoom",
        "Vfp.AssetInspector.Validation.DbfFieldCountZero",
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
        "Vfp.AssetInspector.Validation.ExportTableHasNoFields",
        "Vfp.AssetInspector.Validation.IndexCompanionParseFailed",
        "Vfp.AssetInspector.Validation.IndexStructuralSidecarMissing",
        "Vfp.AssetInspector.Validation.MemoBlockSizeInvalid",
        "Vfp.AssetInspector.Validation.MemoPayloadTruncated",
        "Vfp.AssetInspector.Validation.MemoPointerOutOfRange",
        "Vfp.AssetInspector.Validation.MemoSidecarHeaderTruncated",
        "Vfp.AssetInspector.Validation.MemoSidecarMissing",
        "Vfp.AssetInspector.Validation.MemoSidecarShorterThanBlockSize",
        "Vfp.AssetInspector.Validation.MysqlIdentifierTooLong",
        "Vfp.AssetInspector.Validation.OracleEmptyCharacterValueUnrepresentable",
        "Vfp.AssetInspector.Validation.OracleIdentifierCollision",
        "Vfp.AssetInspector.Validation.UnsafeJsonNumericValue",
        "Vfp.CdxHeader.Error.InvalidValues",
        "Vfp.CdxHeader.Error.OpenFileFailed",
        "Vfp.CdxHeader.Error.ReadProbeFailed",
        "Vfp.CdxHeader.Error.ShortProbe",
        "Vfp.CdxWriter.Error.InvalidKeyLength",
        "Vfp.CdxWriter.Error.InvalidTagName",
        "Vfp.CdxWriter.Error.KeyExceedsDeclaredLength",
        "Vfp.CdxWriter.Error.KeyExpressionTooLong",
        "Vfp.CdxWriter.Error.KeyLengthUnsupported",
        "Vfp.CdxWriter.Error.LeafPageOverflow",
        "Vfp.CdxWriter.Error.OpenFileFailed",
        "Vfp.CdxWriter.Error.RecordNumberOutOfRange",
        "Vfp.CdxWriter.Error.WriteFileFailed",
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

// #5538: real Visual FoxPro stores Stored Procedures source text in a
// catalog row named "StoredProceduresSource", whose CODE memo field holds
// the raw PRG text -- grounded in Microsoft's own archived Visual FoxPro
// Knowledge Base article Q180028, which demonstrates opening a real .dbc
// as a table and directly reading/writing that row's CODE memo (see
// docs/73-vfp-dbc-stored-procedures-and-view-sql.md for the full
// citation). This synthetic fixture reproduces that row shape via
// create_dbf_table_file()'s existing memo-field support (a CODE field
// declared 'M' with real string content automatically becomes a real
// memo block in the generated .dct sidecar, the same mechanism already
// used for the PROPERTIES field in
// test_inspect_database_container_extracts_first_pass_catalog_metadata
// above) rather than committing a real VFP-produced .dbc/.dct pair.
void test_extract_dbc_stored_procedures_source_reads_code_memo() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_stored_procedures_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "withcode.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 0U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 0U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 0U, .length = 32U, .decimal_count = 0U},
        {.name = "CODE", .type = 'M', .offset = 0U, .length = 4U, .decimal_count = 0U},
    };
    const std::string prg_source = "PROCEDURE Greet\n\tLPARAMETERS tcName\n\tRETURN 'Hello ' + tcName\nENDPROC\n";
    const std::vector<std::vector<std::string>> records{
        {"Database", "Northwind", "", ""},
        {"Database", "StoredProceduresSource", "", prg_source},
        // #5544 review (Copilot): "\x03b" would be consumed as a single
        // hex escape (\x03b = 0x3B, not \x03 followed by 'b') -- split
        // into adjacent string literals so the intended 3 binary bytes
        // followed by literal text actually land as written.
        {"Database", "StoredProceduresObject", "", "\x01\x02\x03" "binarycode"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(dbc_path.string(), fields, records);
    expect(create_result.ok, "Stored Procedures DBC fixture creation should succeed: " + create_result.error);

    const auto result = copperfin::vfp::extract_dbc_stored_procedures_source(dbc_path.string());
    expect(result.ok, "extract_dbc_stored_procedures_source should succeed: " + result.error);
    expect(result.available, "extract_dbc_stored_procedures_source should find the StoredProceduresSource row");
    expect(result.source_code == prg_source, "extract_dbc_stored_procedures_source should return the CODE memo's exact text");

    fs::remove_all(temp_dir, ignored);
}

void test_extract_dbc_stored_procedures_source_absent_is_not_an_error() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_stored_procedures_absent_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "nocode.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 0U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 0U, .length = 32U, .decimal_count = 0U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Database", "Northwind"},
        {"Table", "Customers"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(dbc_path.string(), fields, records);
    expect(create_result.ok, "no-stored-procedures DBC fixture creation should succeed: " + create_result.error);

    const auto result = copperfin::vfp::extract_dbc_stored_procedures_source(dbc_path.string());
    expect(result.ok, "extract_dbc_stored_procedures_source should succeed for a database with no stored procedures: " + result.error);
    expect(!result.available, "extract_dbc_stored_procedures_source should report unavailable, not an error, when no StoredProceduresSource row exists");
    expect(result.source_code.empty(), "source_code should stay empty when unavailable");

    fs::remove_all(temp_dir, ignored);
}

void test_extract_dbc_stored_procedures_source_fails_closed_without_memo_sidecar() {
    // A CODE memo pointer with no readable .dct companion (a malformed/
    // incomplete database) must not surface partial or garbage text --
    // matching this issue's acceptance criteria for a fail-closed
    // structured result rather than a guessed reconstruction.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_stored_procedures_no_sidecar_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "missingsidecar.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 0U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 0U, .length = 32U, .decimal_count = 0U},
        {.name = "CODE", .type = 'M', .offset = 0U, .length = 4U, .decimal_count = 0U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Database", "StoredProceduresSource", "PROCEDURE Foo\nENDPROC\n"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(dbc_path.string(), fields, records);
    expect(create_result.ok, "DBC fixture creation should succeed: " + create_result.error);

    const fs::path dct_path = temp_dir / "missingsidecar.dct";
    expect(fs::remove(dct_path, ignored), "the .dct memo sidecar should exist to remove for this test");

    const auto result = copperfin::vfp::extract_dbc_stored_procedures_source(dbc_path.string());
    expect(result.ok, "extract_dbc_stored_procedures_source should still succeed overall: " + result.error);
    expect(!result.available, "a CODE memo pointer with no readable .dct companion should report unavailable rather than garbage text");

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

// #5636: a catalog TABLE object's own name is untrusted data a crafted or
// foreign-tool-written DBC controls, not something this codebase's own
// writer is bound to keep free of path syntax. load_database_catalog_snapshot()
// (shared by every exporter -- JSON, SQL, Access, PostgreSQL, SQLite, SQL
// Server, Oracle, MySQL) previously built each table's own on-disk path via
// plain std::filesystem::path concatenation with no rejection of "..",
// path separators, or an absolute form. Directly reproduced during triage:
// a DBC placed in a "database" subdirectory, with a table named
// "../secret" and a real secret.dbf sitting in that subdirectory's own
// parent, made export_database_as_json() read and disclose the outside
// file's row content verbatim. This test reproduces the exact scenario and
// proves the fix fails the whole export closed instead.
void test_export_database_as_json_rejects_dotdot_table_name_traversal() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_table_traversal_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);
    const fs::path database_dir = temp_dir / "database";
    fs::create_directories(database_dir);

    const fs::path secret_path = temp_dir / "secret.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> secret_fields{
        {.name = "TOKEN", .type = 'C', .offset = 1U, .length = 30U, .decimal_count = 0U}
    };
    const auto secret_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(secret_path), secret_fields,
        {{"OUTSIDE_DATABASE_SECRET"}});
    expect(secret_create.ok, "table traversal test: outside-directory secret fixture should be created");

    const fs::path dbc_path = database_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "../secret", ""}});
    expect(dbc_create.ok, "table traversal test: DBC fixture should be created");

    const auto result = copperfin::vfp::export_database_as_json(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(!result.ok,
           "export_database_as_json should reject a \"../\" catalog table name rather than read outside the database directory");
    expect(result.json.find("OUTSIDE_DATABASE_SECRET") == std::string::npos,
           "export_database_as_json must never leak an outside-directory file's content, even in a failure result");

    fs::remove_all(temp_dir, ignored);
}

// #5636: an absolute-path catalog table name is a second, distinct way to
// hit the same underlying bug -- std::filesystem::path's own operator/
// silently *replaces* the whole left-hand path when the appended component
// is itself absolute, so a table named e.g. "/etc/hostname"-shaped would
// bypass dbc_dir entirely rather than merely escaping it via "..".
void test_export_database_as_json_rejects_absolute_table_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_table_absolute_path_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "/etc/hostname", ""}});
    expect(dbc_create.ok, "absolute table-name test: DBC fixture should be created");

    const auto result = copperfin::vfp::export_database_as_json(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(!result.ok,
           "export_database_as_json should reject an absolute-path catalog table name rather than let it bypass the database directory entirely");

    fs::remove_all(temp_dir, ignored);
}

#if !defined(_WIN32)
// #5636: a table name that passes the string-level safety check (no
// separators, no "..") can still name a symlink planted directly inside
// the database directory that itself points outside it -- the string
// check alone cannot see through that, so the resolved path's own
// canonical form must also be verified contained within the database
// directory's own canonical form.
void test_export_database_as_json_rejects_symlink_table_escaping_directory() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_table_symlink_escape_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);
    const fs::path database_dir = temp_dir / "database";
    fs::create_directories(database_dir);

    const fs::path secret_path = temp_dir / "secret.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> secret_fields{
        {.name = "TOKEN", .type = 'C', .offset = 1U, .length = 30U, .decimal_count = 0U}
    };
    const auto secret_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(secret_path), secret_fields,
        {{"OUTSIDE_DATABASE_SECRET"}});
    expect(secret_create.ok, "symlink escape test: outside-directory secret fixture should be created");

    const fs::path linked_table_path = database_dir / "linked.dbf";
    fs::create_symlink(secret_path, linked_table_path);

    const fs::path dbc_path = database_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "linked", ""}});
    expect(dbc_create.ok, "symlink escape test: DBC fixture should be created");

    const auto result = copperfin::vfp::export_database_as_json(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(!result.ok,
           "export_database_as_json should reject a catalog table whose on-disk file is a symlink escaping the database directory");
    expect(result.json.find("OUTSIDE_DATABASE_SECRET") == std::string::npos,
           "export_database_as_json must never leak the symlink target's own outside-directory content, even in a failure result");

    fs::remove_all(temp_dir, ignored);
}

// #5685 PR review (chatgpt-codex-connector, P1): the previous fix above
// only verified the primary .dbf path's own canonical containment. A
// table's own memo (.fpt) sidecar is resolved independently, later, by
// parse_dbf_table_from_file() -- an in-directory table.dbf sitting
// alongside a table.fpt that is itself a symlink to a memo file outside
// the database directory lets that later, unchecked resolution disclose
// the outside memo content the same way the primary-path check alone was
// meant to prevent. Proves the identical containment check now also
// applies to the resolved memo sidecar.
void test_export_database_as_json_rejects_symlink_memo_sidecar_escaping_directory() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_memo_sidecar_symlink_escape_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);
    const fs::path database_dir = temp_dir / "database";
    fs::create_directories(database_dir);

    // A real memo-backed table outside the database directory, whose own
    // .fpt this test's crafted sidecar will point to.
    const fs::path secret_table_path = temp_dir / "secret.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> secret_fields{
        {.name = "BODY", .type = 'M', .offset = 1U, .length = 4U, .decimal_count = 0U}
    };
    const auto secret_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(secret_table_path), secret_fields,
        {{"OUTSIDE_DATABASE_MEMO_SECRET"}});
    expect(secret_create.ok, "memo sidecar symlink escape test: outside-directory secret fixture should be created");
    const fs::path secret_memo_path = temp_dir / "secret.fpt";
    expect(fs::exists(secret_memo_path),
           "memo sidecar symlink escape test: outside-directory secret fixture should have created a memo sidecar");

    // The in-directory table itself is entirely ordinary and safe -- only
    // its own memo sidecar is a symlink escaping the database directory.
    const fs::path table_path = database_dir / "linked.dbf";
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), secret_fields, {{"harmless"}});
    expect(table_create.ok, "memo sidecar symlink escape test: in-directory table fixture should be created");
    const fs::path table_memo_path = database_dir / "linked.fpt";
    fs::remove(table_memo_path, ignored);
    fs::create_symlink(secret_memo_path, table_memo_path);

    const fs::path dbc_path = database_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "linked", ""}});
    expect(dbc_create.ok, "memo sidecar symlink escape test: DBC fixture should be created");

    const auto result = copperfin::vfp::export_database_as_json(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(!result.ok,
           "export_database_as_json should reject a catalog table whose memo sidecar is a symlink escaping the database directory");
    expect(result.json.find("OUTSIDE_DATABASE_MEMO_SECRET") == std::string::npos,
           "export_database_as_json must never leak the memo symlink target's own outside-directory content, even in a failure result");

    fs::remove_all(temp_dir, ignored);
}
#endif

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

void test_export_database_as_sql_preserves_exponent_form_double_values() {
    // #5545 review (Codex, P1): a VFP 'B' (double) field's decoded
    // display_value comes from an unflagged std::ostringstream at
    // max_digits10 precision (src/vfp/dbf_table.cpp's decode_value(),
    // case 'B'), which switches to scientific notation for sufficiently
    // large/small magnitudes. looks_like_safe_unquoted_sql_numeric_literal()
    // previously only recognized a plain optionally-signed decimal, so
    // any such value was silently replaced with NULL -- real (not
    // merely crafted-input) data loss. This proves a genuinely large
    // double round-trips through export_database_as_sql() as a real
    // unquoted numeric literal rather than NULL.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sql_exponent_double_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "measurements.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "measurements", ""}});
    expect(dbc_create.ok, "exponent-double test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "READING", .type = 'B', .offset = 1U, .length = 8U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"1e100"}});
    expect(table_create.ok, "exponent-double test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sql should resolve the measurements fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("VALUES (NULL)") == std::string::npos,
           "export_database_as_sql must not silently replace a legitimate exponent-form double with NULL");
    const bool has_exponent_literal =
        result.sql.find("e+") != std::string::npos || result.sql.find("E+") != std::string::npos ||
        result.sql.find("e100") != std::string::npos || result.sql.find("E100") != std::string::npos;
    expect(has_exponent_literal,
           "export_database_as_sql should emit the exponent-form double value unquoted, not drop its magnitude");

    fs::remove_all(temp_dir, ignored);
}

// #5630 (found by an automated Codex code-review pass): export_database_as_
// json() previously inserted a numeric cell's decoded display_value
// directly into the JSON stream with no validation at all. A crafted or
// corrupted DBF can contain arbitrary byte content in a fixed-width N/F
// field (this codebase's own DBF writer accepts any byte string that fits
// the declared width), so a value like `1,"INJECT":true` didn't just
// produce invalid JSON -- it injected an entirely new, distinct JSON
// property into the record object while the export still reported
// success. Verified to reliably fail against the pre-fix code (which
// emitted the injected property and returned ok=true) and reliably pass
// against the fix.
void test_export_database_as_json_fails_closed_on_numeric_structural_injection() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_json_numeric_injection_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "amounts.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "amounts", ""}});
    expect(dbc_create.ok, "JSON numeric-injection test: DBC fixture should be created");

    const std::string injected_value = "1,\"INJECT\":true";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "AMOUNT", .type = 'N', .offset = 1U, .length = static_cast<std::uint8_t>(injected_value.size()), .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{injected_value}});
    expect(table_create.ok, "JSON numeric-injection test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_json(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(!result.ok,
           "export_database_as_json must fail closed on a numeric cell that would inject JSON structure rather than report success");
    expect(result.json.find("INJECT") == std::string::npos,
           "export_database_as_json must never emit the injected content, even in a failure result");

    fs::remove_all(temp_dir, ignored);
}

// #5571/#5630: covers the remaining unsafe-numeric-form cases the JSON
// exporter previously accepted verbatim: a classic dBASE-family
// numeric-overflow marker (invalid JSON outright), a leading '+' sign
// (valid in this codebase's own SQL-literal grammar but not in real JSON
// number grammar, RFC 8259), a leading zero followed by further digits
// (likewise SQL-safe but JSON-invalid), and a nonfinite binary Double's
// own decoded text ("nan", not a valid JSON token). All four must fail
// the export closed rather than emit invalid or silently-reinterpreted
// JSON. Verified to reliably fail against the pre-fix code and reliably
// pass against the fix.
void test_export_database_as_json_fails_closed_on_unsafe_numeric_forms() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_json_unsafe_numeric_forms_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };

    const auto check_fails_closed = [&](const char* case_name, char field_type,
                                         std::uint8_t field_length, const std::string& value) {
        // Each case gets its own subdirectory so the table file can keep
        // the plain "readings.dbf" name the DBC catalog's own OBJECTNAME
        // ("readings") resolves against -- a case-prefixed table filename
        // would leave the catalog's table entry unresolved, and the export
        // would then trivially "succeed" with zero resolved tables rather
        // than actually exercising the numeric-safety check at all.
        const fs::path case_dir = temp_dir / case_name;
        std::error_code case_ignored;
        fs::create_directories(case_dir, case_ignored);
        const fs::path dbc_path = case_dir / "container.dbc";
        const fs::path table_path = case_dir / "readings.dbf";
        const auto dbc_create = copperfin::vfp::create_dbf_table_file(
            copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
            {{"TABLE", "readings", ""}});
        expect(dbc_create.ok, std::string(case_name) + ": DBC fixture should be created");

        const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
            {.name = "VALUE", .type = field_type, .offset = 1U, .length = field_length, .decimal_count = 0U},
        };
        const auto table_create = copperfin::vfp::create_dbf_table_file(
            copperfin::platform::path_to_utf8_string(table_path), table_fields, {{value}});
        expect(table_create.ok, std::string(case_name) + ": DBF fixture should be created");

        const auto result = copperfin::vfp::export_database_as_json(
            copperfin::platform::path_to_utf8_string(dbc_path));
        expect(!result.ok, std::string("export_database_as_json must fail closed for case: ") + case_name);
    };

    check_fails_closed("overflow_marker", 'N', 5U, "*****");
    check_fails_closed("leading_plus", 'N', 4U, "+123");
    check_fails_closed("leading_zero_digit", 'N', 4U, "0123");
    check_fails_closed("nonfinite_double", 'B', 8U, "nan");

    fs::remove_all(temp_dir, ignored);
}

// A genuinely valid numeric value (plain integer, negative, fractional,
// exponent form, or "0") must still export successfully and round-trip
// as a real unquoted JSON number -- #5630's own fix must not become
// over-broad and reject legitimate values.
void test_export_database_as_json_still_accepts_valid_numeric_forms() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_json_valid_numeric_forms_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "readings.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "readings", ""}});
    expect(dbc_create.ok, "JSON valid-numeric test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "VALUE", .type = 'N', .offset = 1U, .length = 10U, .decimal_count = 2U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"42"}, {"-3.14"}, {"0"}, {""}});
    expect(table_create.ok, "JSON valid-numeric test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_json(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_json should still succeed for genuinely valid numeric values: " + result.error);
    if (result.ok) {
        expect(result.json.find("\"VALUE\": 42") != std::string::npos,
               "export_database_as_json should emit a plain integer unquoted");
        expect(result.json.find("\"VALUE\": -3.14") != std::string::npos,
               "export_database_as_json should emit a negative fractional value unquoted");
        expect(result.json.find("\"VALUE\": 0") != std::string::npos,
               "export_database_as_json should emit a bare zero unquoted");
        const auto plan = copperfin::vfp::build_database_json_import_plan(result.json);
        expect(plan.ok, "the resulting JSON should itself be valid enough for the import planner to parse: " + plan.error_code);
    }

    fs::remove_all(temp_dir, ignored);
}

// #5630 review (self, proactive sibling-gap check performed while fixing
// the numeric-value injection this issue reports): the fields array's own
// "type" property embeds a field descriptor's raw type byte directly with
// no validation at all -- unlike the field's own "name" property, which
// already routes through json_escape_str(). parse_dbf_field_descriptor_
// block() never validates that this byte is one of the recognized VFP
// type letters, so a crafted/corrupted DBF can set it to a literal '"' or
// '\' and break the fields array's own JSON structure exactly like an
// unvalidated numeric display_value does for a data value -- a different
// injection point in the same function, found by checking for sibling
// gaps near the fix rather than waiting for a separate report. Fixture
// built from raw DBF bytes directly (an out-of-range type byte cannot be
// produced through the ordinary create_dbf_table_file() writer, which
// only ever accepts a fixed, real set of VFP type letters). Verified to
// reliably fail (invalid JSON containing an unescaped `"""` sequence)
// against the pre-fix code and reliably pass against the fix.
void test_export_database_as_json_escapes_crafted_field_type_byte() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_json_crafted_field_type_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "readings.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "readings", ""}});
    expect(dbc_create.ok, "crafted field-type test: DBC fixture should be created");

    // Raw VFP-style DBF bytes: one field descriptor whose type byte
    // (offset 11 within the descriptor) is a literal '"' rather than a
    // real VFP type letter.
    constexpr std::uint16_t record_length = 1U + 10U;
    constexpr std::uint16_t header_length = 32U + 32U + 1U;
    std::vector<std::uint8_t> bytes(
        static_cast<std::size_t>(header_length) + record_length + 1U, 0U);
    bytes[0] = 0x30U;
    write_le_u32(bytes, 4U, 1U);
    write_le_u16(bytes, 8U, header_length);
    write_le_u16(bytes, 10U, record_length);
    write_ascii(bytes, 32U, "VALUE");
    bytes[32U + 11U] = '"';  // crafted field-type byte
    bytes[32U + 16U] = 10U;  // length
    bytes[64U] = 0x0DU;      // field descriptor terminator
    bytes[header_length] = 0x20U;  // not deleted
    std::memset(bytes.data() + header_length + 1U, ' ', 10U);
    bytes.back() = 0x1AU;  // EOF marker
    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_json(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_json should still succeed once the type byte is properly escaped: " + result.error);
    if (result.ok) {
        expect(result.json.find("\"type\": \"\\\"\"") != std::string::npos,
               "export_database_as_json should emit the crafted type byte as a properly escaped JSON string");
        // Only the JSON syntax itself is under test here -- a raw '"' is
        // not a real VFP field type letter, so build_database_json_import_
        // plan() separately and correctly rejects it as an invalid field
        // type on its own semantic grounds; that's an unrelated, already
        // fail-closed concern, not a claim this exact document should be
        // re-importable.
        const auto plan = copperfin::vfp::build_database_json_import_plan(result.json);
        expect(plan.error_code != "database_json_import.invalid_document",
               "the resulting JSON must at least parse as syntactically valid JSON (a structural-injection regression would fail parsing entirely, not just field-type semantics): " +
                   plan.error_code);
    }

    fs::remove_all(temp_dir, ignored);
}

// #5696: a blank VFP date field decodes to an empty display_value (not
// is_null), same as a blank numeric cell -- write_sql_tables_and_data()
// (the shared row writer behind export_database_as_sql(),
// export_database_as_postgresql_sql(), and export_database_as_sqlite_sql())
// had no 'D'/Date branch at all, so a blank date fell through to the
// generic string path and was emitted as a plain `''` literal into a
// column declared DATE. Real PostgreSQL rejects `''` as invalid DATE
// input outright; SQLite's dynamic typing accepts the statement but
// stores a TEXT value in the DATE/NUMERIC-affinity column, silently
// preserving the wrong SQL storage class. Mirrors the fix already
// applied to the SQL Server/Oracle/MySQL writers (see
// test_export_database_as_sqlserver_sql_preserves_blank_dates_as_null()).
// Verified to reliably fail against the pre-fix code (missing is_date
// branch) and reliably pass against the fix, for all three exporters
// that share write_sql_tables_and_data().
void test_export_database_as_sql_family_preserves_blank_dates_as_null() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sql_family_blank_date_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "events.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "events", ""}});
    expect(dbc_create.ok, "SQL family blank-date test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "OCCURRED", .type = 'D', .offset = 1U, .length = 8U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"20260115"}, {""}});
    expect(table_create.ok, "SQL family blank-date test: DBF fixture should be created");

    const std::string dbc_utf8 = copperfin::platform::path_to_utf8_string(dbc_path);

    const auto plain_result = copperfin::vfp::export_database_as_sql(dbc_utf8);
    expect(plain_result.ok, "export_database_as_sql should resolve the blank-date fixture: " + plain_result.error);
    if (plain_result.ok) {
        expect(plain_result.sql.find("VALUES ('2026-01-15')") != std::string::npos,
               "export_database_as_sql should emit a non-blank date as a normal quoted string literal");
        expect(plain_result.sql.find("VALUES (NULL)") != std::string::npos,
               "export_database_as_sql should emit NULL for a blank date rather than an empty string literal");
        expect(plain_result.sql.find("VALUES ('')") == std::string::npos,
               "export_database_as_sql must never emit an empty string literal for a DATE column");
    }

    const auto postgresql_result = copperfin::vfp::export_database_as_postgresql_sql(dbc_utf8);
    expect(postgresql_result.ok,
           "export_database_as_postgresql_sql should resolve the blank-date fixture: " + postgresql_result.error);
    if (postgresql_result.ok) {
        expect(postgresql_result.sql.find("VALUES ('2026-01-15')") != std::string::npos,
               "export_database_as_postgresql_sql should emit a non-blank date as a normal quoted string literal");
        expect(postgresql_result.sql.find("VALUES (NULL)") != std::string::npos,
               "export_database_as_postgresql_sql should emit NULL for a blank date rather than an empty string literal");
        expect(postgresql_result.sql.find("VALUES ('')") == std::string::npos,
               "export_database_as_postgresql_sql must never emit an empty string literal for a DATE column -- real PostgreSQL rejects it as invalid date input");
    }

    const auto sqlite_result = copperfin::vfp::export_database_as_sqlite_sql(dbc_utf8);
    expect(sqlite_result.ok,
           "export_database_as_sqlite_sql should resolve the blank-date fixture: " + sqlite_result.error);
    if (sqlite_result.ok) {
        expect(sqlite_result.sql.find("VALUES ('2026-01-15')") != std::string::npos,
               "export_database_as_sqlite_sql should emit a non-blank date as a normal quoted string literal");
        expect(sqlite_result.sql.find("VALUES (NULL)") != std::string::npos,
               "export_database_as_sqlite_sql should emit NULL for a blank date rather than an empty string literal");
        expect(sqlite_result.sql.find("VALUES ('')") == std::string::npos,
               "export_database_as_sqlite_sql must never emit an empty string literal for a DATE column -- it would silently store the wrong SQL storage class under dynamic typing");
    }

    fs::remove_all(temp_dir, ignored);
}

// #5537: a dedicated two-tag synthetic CDX, mirroring
// make_synthetic_cdx_family_bytes()'s own verified byte layout (same tag-
// entry/tag-page structure and offsets), but with key expressions that
// are themselves legal real VFP field names (<= 10 bytes,
// dbf_free_table_field_name_max_bytes in src/vfp/dbf_table.cpp) for the
// plain-column case -- make_synthetic_cdx_family_bytes()'s own
// "customer_id"/"company_name" strings exceed that real limit, since
// those existing tests only exercise CDX-format parsing in isolation,
// decoupled from any real DBF field-name constraint.
std::vector<std::uint8_t> make_synthetic_cdx_bytes_for_postgresql_index_test() {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(bytes, 1026U, 2U);
    write_le_u32(bytes, 1028U, 11U * 512U);
    write_le_u32(bytes, 1032U, 4U * 512U);
    write_le_u16(bytes, 11U * 512U, 0x0001U);
    write_le_u16(bytes, (11U * 512U) + 2U, 1U);
    write_le_u16(bytes, 4U * 512U, 0x0003U);
    write_le_u16(bytes, (4U * 512U) + 2U, 2U);
    write_ascii(bytes, (3U * 512U) - 20U, "CUST_ID");
    write_ascii(bytes, (3U * 512U) - 10U, "COMPANY_N");
    write_ascii(bytes, (4U * 512U) + 24U, "UPPER(company_name)");
    write_ascii(bytes, (11U * 512U) + 24U, "cust_id");
    return bytes;
}

void test_export_database_as_postgresql_sql_maps_types_and_creates_indexes() {
    // #5537 (parent #137, first vendor-dialect slice): real PostgreSQL
    // already accepts export_database_as_sql()'s double-quoted
    // identifiers, single-quoted string literals, and DECIMAL/INTEGER/
    // DOUBLE PRECISION/BOOLEAN/DATE/TIMESTAMP/VARCHAR/TEXT column types
    // verbatim (per PostgreSQL's own public SQL/DDL documentation), so
    // this proves export_database_as_postgresql_sql()'s genuinely new
    // piece: CREATE INDEX statements derived from a table's production
    // CDX tags. The synthetic CDX's two tags are exactly the "plain
    // column reference" and "composite expression" cases this exporter
    // must tell apart: CUST_ID -> "cust_id" (a plain column), COMPANY_N
    // -> "UPPER(company_name)" (not translatable to a single-column
    // index in this first slice).
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_postgresql_sql_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "customers.dbf";
    const fs::path cdx_path = temp_dir / "customers.cdx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"DATABASE", "Sales", "", ""}, {"TABLE", "customers", "Sales", ""}});
    expect(dbc_create.ok, "PostgreSQL export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "CUST_ID", .type = 'N', .offset = 1U, .length = 6U, .decimal_count = 0U},
        {.name = "COMPANY", .type = 'C', .offset = 7U, .length = 40U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"1001", "Acme Corp"}});
    expect(table_create.ok, "PostgreSQL export test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_cdx_bytes_for_postgresql_index_test();
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_postgresql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_postgresql_sql should resolve the customers fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("CREATE TABLE \"customers\"") != std::string::npos,
           "export_database_as_postgresql_sql should quote identifiers with double quotes, matching real PostgreSQL");
    expect(result.sql.find("\"CUST_ID\" DECIMAL(6, 0)") != std::string::npos,
           "export_database_as_postgresql_sql should map a numeric field to DECIMAL, a real PostgreSQL type");
    expect(result.sql.find("\"COMPANY\" VARCHAR(40)") != std::string::npos,
           "export_database_as_postgresql_sql should map a character field to VARCHAR(length)");
    expect(result.sql.find("INSERT INTO \"customers\"") != std::string::npos,
           "export_database_as_postgresql_sql should emit an INSERT for the table's row");
    expect(result.sql.find("'Acme Corp'") != std::string::npos,
           "export_database_as_postgresql_sql should quote a character value as a standard SQL string literal");

    expect(result.sql.find("CREATE INDEX \"customers_CUST_ID_idx\" ON \"customers\" (\"CUST_ID\");") != std::string::npos,
           "export_database_as_postgresql_sql should emit a CREATE INDEX for a tag whose key expression is a plain column reference");
    expect(result.sql.find("-- skipped index") != std::string::npos &&
               result.sql.find("COMPANY_N") != std::string::npos,
           "export_database_as_postgresql_sql should report a composite-expression tag as a skipped index, not silently drop or mistranslate it");
    expect(result.sql.find("UPPER(company_name)") == std::string::npos,
           "export_database_as_postgresql_sql must never emit a raw VFP key expression as if it were valid PostgreSQL syntax");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_sqlite_sql_maps_types_and_creates_indexes() {
    // #5554 (parent #137, second vendor-dialect slice, following #5537's
    // PostgreSQL precedent): real SQLite (3.46.1) directly confirmed to
    // accept this exact dialect -- double-quoted identifiers, single-
    // quoted string literals, DECIMAL/VARCHAR/BOOLEAN/DATE/TIMESTAMP/
    // TEXT column types (SQLite's own type-affinity rules bucket every
    // one of these by substring match rather than rejecting them) --
    // loading a representative CREATE TABLE/INSERT/CREATE INDEX script
    // shaped exactly like this exporter's own output without error
    // during this issue's own development. This test proves the same
    // genuinely new piece #5537's own sibling test proves: CREATE INDEX
    // statements derived from a table's production CDX tags, correctly
    // distinguishing a plain column reference from a composite
    // expression that cannot translate to a single-column index.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlite_sql_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "customers.dbf";
    const fs::path cdx_path = temp_dir / "customers.cdx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"DATABASE", "Sales", "", ""}, {"TABLE", "customers", "Sales", ""}});
    expect(dbc_create.ok, "SQLite export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "CUST_ID", .type = 'N', .offset = 1U, .length = 6U, .decimal_count = 0U},
        {.name = "COMPANY", .type = 'C', .offset = 7U, .length = 40U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"1001", "Acme Corp"}});
    expect(table_create.ok, "SQLite export test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_cdx_bytes_for_postgresql_index_test();
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_sqlite_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlite_sql should resolve the customers fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("EXPORT DATABASE ... TYPE SQLITE") != std::string::npos,
           "export_database_as_sqlite_sql should label its own header comment as a SQLite export");
    expect(result.sql.find("CREATE TABLE \"customers\"") != std::string::npos,
           "export_database_as_sqlite_sql should quote identifiers with double quotes, matching real SQLite");
    expect(result.sql.find("\"CUST_ID\" DECIMAL(6, 0)") != std::string::npos,
           "export_database_as_sqlite_sql should map a numeric field to DECIMAL, which SQLite accepts under NUMERIC affinity");
    expect(result.sql.find("\"COMPANY\" VARCHAR(40)") != std::string::npos,
           "export_database_as_sqlite_sql should map a character field to VARCHAR(length), which SQLite accepts under TEXT affinity");
    expect(result.sql.find("INSERT INTO \"customers\"") != std::string::npos,
           "export_database_as_sqlite_sql should emit an INSERT for the table's row");
    expect(result.sql.find("'Acme Corp'") != std::string::npos,
           "export_database_as_sqlite_sql should quote a character value as a standard SQL string literal");

    expect(result.sql.find("CREATE INDEX \"customers_CUST_ID_idx\" ON \"customers\" (\"CUST_ID\");") != std::string::npos,
           "export_database_as_sqlite_sql should emit a CREATE INDEX for a tag whose key expression is a plain column reference");
    expect(result.sql.find("-- skipped index") != std::string::npos &&
               result.sql.find("COMPANY_N") != std::string::npos,
           "export_database_as_sqlite_sql should report a composite-expression tag as a skipped index, not silently drop or mistranslate it");
    expect(result.sql.find("UPPER(company_name)") == std::string::npos,
           "export_database_as_sqlite_sql must never emit a raw VFP key expression as if it were valid SQLite syntax");

    fs::remove_all(temp_dir, ignored);
}

// #5558 review (chatgpt-codex-connector): SQLite index names are
// schema-wide, not scoped to their own table -- a table named "A_B"
// with a tag named "CDEF" and a table named "A" with a tag named
// "B_CDEF" both concatenate to the identical "A_B_CDEF_idx" under the
// naming scheme write_postgresql_create_indexes() already established,
// and real SQLite rejects the second CREATE INDEX for an already-
// existing name. Mirrors make_synthetic_cdx_bytes_for_postgresql_
// index_test()'s own proven layout with a parameterized "real" tag
// (kept a second, always-skipped composite-expression tag as a decoy,
// matching that fixture's own structure) so each table gets its own
// single-tag CDX. Tag names must satisfy cdx_header.cpp's own
// looks_like_tag_name_candidate() (4-10 identifier characters,
// containing at least one uppercase letter) to be recognized at all.
std::vector<std::uint8_t> make_synthetic_single_tag_cdx_bytes(
    const std::string& tag_name, const std::string& key_expression) {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(bytes, 1026U, 2U);
    write_le_u32(bytes, 1028U, 11U * 512U);
    write_le_u32(bytes, 1032U, 4U * 512U);
    write_le_u16(bytes, 11U * 512U, 0x0001U);
    write_le_u16(bytes, (11U * 512U) + 2U, 1U);
    write_le_u16(bytes, 4U * 512U, 0x0003U);
    write_le_u16(bytes, (4U * 512U) + 2U, 2U);
    write_ascii(bytes, (3U * 512U) - 20U, tag_name.c_str());
    write_ascii(bytes, (3U * 512U) - 10U, "COMPANY_N");
    write_ascii(bytes, (4U * 512U) + 24U, "UPPER(company_name)");
    write_ascii(bytes, (11U * 512U) + 24U, key_expression.c_str());
    return bytes;
}

void test_export_database_as_sqlite_sql_disambiguates_indexes_across_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlite_sql_cross_table_collision_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "A_B", ""}, {"TABLE", "A", ""}});
    expect(dbc_create.ok, "SQLite cross-table index-collision test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table1_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A_B.dbf").string(), table_fields, {{"x"}});
    expect(table1_create.ok, "SQLite cross-table index-collision test: A_B.dbf fixture should be created");
    const auto table2_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A.dbf").string(), table_fields, {{"y"}});
    expect(table2_create.ok, "SQLite cross-table index-collision test: A.dbf fixture should be created");

    // Table "A_B"'s tag "CDEF" and table "A"'s tag "B_CDEF" both
    // concatenate to "A_B_CDEF_idx" under the plain <table>_<tag>_idx
    // naming scheme -- real CDX tag names must be 4-10 identifier
    // characters containing at least one uppercase letter
    // (cdx_header.cpp's own looks_like_tag_name_candidate()), which
    // "B_CDEF" (6 chars, one embedded underscore) satisfies.
    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("CDEF", "COL");
        std::ofstream output(temp_dir / "A_B.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }
    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("B_CDEF", "COL");
        std::ofstream output(temp_dir / "A.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_sqlite_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlite_sql should resolve the cross-table collision fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    const std::size_t first_occurrence = result.sql.find("CREATE INDEX \"A_B_CDEF_idx\"");
    expect(first_occurrence != std::string::npos,
           "export_database_as_sqlite_sql should emit the first table's own naturally-derived index name unchanged");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx\"", first_occurrence + 1U) == std::string::npos,
           "export_database_as_sqlite_sql must never emit the exact same index name twice across different tables");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx_2\"") != std::string::npos,
           "export_database_as_sqlite_sql should disambiguate the second table's colliding index name with a deterministic suffix");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_sqlite_sql_omits_indexes_without_cdx() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlite_sql_no_cdx_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "SQLite no-CDX export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "SKU", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"W-1"}});
    expect(table_create.ok, "SQLite no-CDX export test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_sqlite_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlite_sql should succeed without a companion CDX: " + result.error);
    expect(result.sql.find("CREATE TABLE \"widgets\"") != std::string::npos,
           "export_database_as_sqlite_sql should still emit the table without any index information");
    expect(result.sql.find("CREATE INDEX") == std::string::npos,
           "export_database_as_sqlite_sql should not emit any CREATE INDEX when no companion CDX exists");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_postgresql_sql_omits_indexes_without_cdx() {
    // A table with no companion .cdx (or none at the conventional same-
    // base-name path) should still export cleanly -- no CREATE INDEX
    // statements, no error, matching this exporter's read-only,
    // best-effort index-derivation scope.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_postgresql_sql_no_cdx_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "PostgreSQL no-CDX export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "SKU", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"W-1"}});
    expect(table_create.ok, "PostgreSQL no-CDX export test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_postgresql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_postgresql_sql should succeed without a companion CDX: " + result.error);
    expect(result.sql.find("CREATE TABLE \"widgets\"") != std::string::npos,
           "export_database_as_postgresql_sql should still emit the table without any index information");
    expect(result.sql.find("CREATE INDEX") == std::string::npos,
           "export_database_as_postgresql_sql should not emit any CREATE INDEX when no companion CDX exists");

    fs::remove_all(temp_dir, ignored);
}

// #5545 review (Copilot and Codex, independently): two distinct tags over
// the same plain column (e.g. differing only in sort order/filter) must
// not generate the identical `<table>_<column>_idx` name -- PostgreSQL
// rejects a second CREATE INDEX for an already-existing relation name,
// leaving an otherwise advertised runnable export incomplete.
std::vector<std::uint8_t> make_synthetic_cdx_bytes_with_two_tags_on_same_column() {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(bytes, 1026U, 2U);
    write_le_u32(bytes, 1028U, 11U * 512U);
    write_le_u32(bytes, 1032U, 4U * 512U);
    write_le_u16(bytes, 11U * 512U, 0x0001U);
    write_le_u16(bytes, (11U * 512U) + 2U, 1U);
    write_le_u16(bytes, 4U * 512U, 0x0003U);
    write_le_u16(bytes, (4U * 512U) + 2U, 2U);
    write_ascii(bytes, (3U * 512U) - 20U, "SKU_ASC");
    write_ascii(bytes, (3U * 512U) - 10U, "SKU_DESC");
    write_ascii(bytes, (4U * 512U) + 24U, "sku");
    write_ascii(bytes, (11U * 512U) + 24U, "sku");
    return bytes;
}

// Parameterized sibling of make_synthetic_cdx_bytes_with_two_tags_on_same_column()
// above, letting a test choose its own (short, real-tag-name-valid) tag
// names and shared key expression -- used to prove the 63-byte
// PostgreSQL identifier truncation case below, where the *table* name
// itself (chosen by the test, not this fixture) supplies the long
// common prefix.
std::vector<std::uint8_t> make_synthetic_two_tag_cdx_bytes(
    const std::string& tag1_name, const std::string& tag2_name, const std::string& key_expression) {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(bytes, 1026U, 2U);
    write_le_u32(bytes, 1028U, 11U * 512U);
    write_le_u32(bytes, 1032U, 4U * 512U);
    write_le_u16(bytes, 11U * 512U, 0x0001U);
    write_le_u16(bytes, (11U * 512U) + 2U, 1U);
    write_le_u16(bytes, 4U * 512U, 0x0003U);
    write_le_u16(bytes, (4U * 512U) + 2U, 2U);
    write_ascii(bytes, (3U * 512U) - 20U, tag1_name.c_str());
    write_ascii(bytes, (3U * 512U) - 10U, tag2_name.c_str());
    write_ascii(bytes, (4U * 512U) + 24U, key_expression.c_str());
    write_ascii(bytes, (11U * 512U) + 24U, key_expression.c_str());
    return bytes;
}

// #5559 review (chatgpt-codex-connector): PostgreSQL silently truncates
// any identifier over NAMEDATALEN-1 (63) bytes to that length, so two
// distinct candidate names that agree only in their first 63 bytes
// still collide inside the *real* engine even though this exporter's
// own untruncated bookkeeping would see them as unique -- and a
// candidate that needs a numeric suffix must have that suffix land
// inside the 63-byte budget, not be silently truncated away itself.
// Table "AAAA...A" (60 'A' characters) with tags "TAG1"/"TAG2" on the
// same column produces raw candidates "AAAA...A_TAG1_idx" and
// "AAAA...A_TAG2_idx" (69 bytes each) that are identical for their
// first 63 bytes (index 63 is where "TAG1" and "TAG2" first differ),
// so both must truncate to the same 63-byte name before this exporter's
// own uniqueness check can catch the collision and suffix the second
// one within the same 63-byte budget.
void test_export_database_as_postgresql_sql_disambiguates_indexes_within_identifier_length_limit() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_postgresql_sql_identifier_length_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::string long_table_name(60U, 'A');
    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / (long_table_name + ".dbf");
    const fs::path cdx_path = temp_dir / (long_table_name + ".cdx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", long_table_name, ""}});
    expect(dbc_create.ok, "PostgreSQL identifier-length test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x"}});
    expect(table_create.ok, "PostgreSQL identifier-length test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_two_tag_cdx_bytes("TAG1", "TAG2", "COL");
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_postgresql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_postgresql_sql should resolve the identifier-length fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    // First candidate truncates to exactly 63 bytes: the 60 'A's plus
    // "_TA" (the first two characters after the underscore that "TAG1"
    // and "TAG2" still share before diverging at what would be index 63).
    const std::string first_truncated = long_table_name + "_TA";
    expect(first_truncated.size() == 63U,
           "test fixture sanity: the first truncated candidate should be exactly 63 bytes");
    expect(result.sql.find("CREATE INDEX \"" + first_truncated + "\"") != std::string::npos,
           "export_database_as_postgresql_sql should truncate the first candidate to PostgreSQL's 63-byte identifier limit");

    // Second candidate collides with the first *after* truncation, so it
    // must be suffixed -- and that suffix must itself fit inside the
    // same 63-byte budget rather than being silently truncated away.
    const std::string second_suffixed = long_table_name + "__2";
    expect(second_suffixed.size() == 63U,
           "test fixture sanity: the disambiguated second candidate should still be exactly 63 bytes");
    expect(result.sql.find("CREATE INDEX \"" + second_suffixed + "\"") != std::string::npos,
           "export_database_as_postgresql_sql should reserve room for the disambiguating suffix within the 63-byte limit");
    expect(result.sql.find("CREATE INDEX \"" + first_truncated + "\"", result.sql.find("CREATE INDEX \"" + first_truncated + "\"") + 1U)
               == std::string::npos,
           "export_database_as_postgresql_sql must never emit the exact same truncated index name twice");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_postgresql_sql_disambiguates_indexes_on_same_column() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_postgresql_sql_duplicate_column_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const fs::path cdx_path = temp_dir / "widgets.cdx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "PostgreSQL duplicate-column-index test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "SKU", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"W-1"}});
    expect(table_create.ok, "PostgreSQL duplicate-column-index test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_cdx_bytes_with_two_tags_on_same_column();
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_postgresql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_postgresql_sql should succeed: " + result.error);

    expect(result.sql.find("CREATE INDEX \"widgets_SKU_ASC_idx\" ON \"widgets\" (\"SKU\");") != std::string::npos,
           "export_database_as_postgresql_sql should name an index after its own CDX tag, not just table+column");
    expect(result.sql.find("CREATE INDEX \"widgets_SKU_DESC_idx\" ON \"widgets\" (\"SKU\");") != std::string::npos,
           "export_database_as_postgresql_sql should emit a second, distinctly-named CREATE INDEX for the second tag on the same column");

    fs::remove_all(temp_dir, ignored);
}

// #5559: write_postgresql_create_indexes() had the identical cross-table
// collision bug fixed for SQLite in the #5558 review -- see
// test_export_database_as_sqlite_sql_disambiguates_indexes_across_tables()'s
// own comment for the fixture rationale. Real PostgreSQL index names are
// schema-wide, not per-table, so the same "A_B"/tag "CDEF" vs. "A"/tag
// "B_CDEF" collision applies unchanged.
void test_export_database_as_postgresql_sql_disambiguates_indexes_across_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_postgresql_sql_cross_table_collision_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "A_B", ""}, {"TABLE", "A", ""}});
    expect(dbc_create.ok, "PostgreSQL cross-table index-collision test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table1_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A_B.dbf").string(), table_fields, {{"x"}});
    expect(table1_create.ok, "PostgreSQL cross-table index-collision test: A_B.dbf fixture should be created");
    const auto table2_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A.dbf").string(), table_fields, {{"y"}});
    expect(table2_create.ok, "PostgreSQL cross-table index-collision test: A.dbf fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("CDEF", "COL");
        std::ofstream output(temp_dir / "A_B.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }
    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("B_CDEF", "COL");
        std::ofstream output(temp_dir / "A.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_postgresql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_postgresql_sql should resolve the cross-table collision fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    const std::size_t first_occurrence = result.sql.find("CREATE INDEX \"A_B_CDEF_idx\"");
    expect(first_occurrence != std::string::npos,
           "export_database_as_postgresql_sql should emit the first table's own naturally-derived index name unchanged");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx\"", first_occurrence + 1U) == std::string::npos,
           "export_database_as_postgresql_sql must never emit the exact same index name twice across different tables");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx_2\"") != std::string::npos,
           "export_database_as_postgresql_sql should disambiguate the second table's colliding index name with a deterministic suffix");

    fs::remove_all(temp_dir, ignored);
}

// #5559 review (chatgpt-codex-connector): PostgreSQL puts tables and
// indexes in the same schema-wide relation namespace, not just indexes
// against each other -- a catalog table literally named "A_B_CDEF_idx"
// (a valid, if unusual, VFP table name) collides with the index this
// exporter would otherwise emit unchanged for table "A_B"'s tag "CDEF".
// export_database_as_postgresql_sql() must seed its disambiguation set
// with every table name it has already emitted a CREATE TABLE for, not
// just with previously-emitted index names.
void test_export_database_as_postgresql_sql_disambiguates_index_colliding_with_table_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_postgresql_sql_table_index_namespace_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "A_B", ""}, {"TABLE", "A_B_CDEF_idx", ""}});
    expect(dbc_create.ok, "PostgreSQL table/index namespace test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table1_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A_B.dbf").string(), table_fields, {{"x"}});
    expect(table1_create.ok, "PostgreSQL table/index namespace test: A_B.dbf fixture should be created");
    const auto table2_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A_B_CDEF_idx.dbf").string(), table_fields, {{"y"}});
    expect(table2_create.ok, "PostgreSQL table/index namespace test: A_B_CDEF_idx.dbf fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("CDEF", "COL");
        std::ofstream output(temp_dir / "A_B.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }
    // "A_B_CDEF_idx" carries no companion .cdx of its own -- it exists
    // purely to occupy the relation name the other table's index would
    // otherwise collide with.

    const auto result = copperfin::vfp::export_database_as_postgresql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_postgresql_sql should resolve the table/index namespace fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("CREATE TABLE \"A_B_CDEF_idx\"") != std::string::npos,
           "export_database_as_postgresql_sql should still emit the oddly-named table itself");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx\"") == std::string::npos,
           "export_database_as_postgresql_sql must not emit a CREATE INDEX whose name collides with an already-emitted table name");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx_2\"") != std::string::npos,
           "export_database_as_postgresql_sql should disambiguate the index name that collides with a table name");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_sqlserver_sql_maps_types_and_creates_indexes() {
    // #5554 (parent #137, third vendor-dialect slice, following #5537's
    // PostgreSQL and #5558's SQLite precedent): real SQL Server 2022
    // (Developer Edition, verified against a local container during
    // this issue's own development) directly confirmed to accept this
    // exact dialect -- square-bracket identifiers, single-quoted string
    // literals, and DECIMAL/MONEY/INT/FLOAT/BIT/DATE/DATETIME2/
    // VARCHAR(length)/VARCHAR(MAX) column types -- loading a
    // representative CREATE TABLE/INSERT/CREATE INDEX script shaped
    // exactly like this exporter's own output without error, including
    // a cross-table JOIN returning the correct row. This test proves the
    // same genuinely new piece #5537's own sibling test proves (CREATE
    // INDEX statements derived from a table's production CDX tags,
    // correctly distinguishing a plain column reference from a
    // composite expression), plus the BIT column's 1/0 literal
    // requirement: real T-SQL rejects `INSERT ... VALUES (TRUE)` with
    // "Invalid column name 'TRUE'" (T-SQL has no boolean-literal syntax
    // outside a predicate context), directly confirmed against the same
    // real engine.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlserver_sql_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "customers.dbf";
    const fs::path cdx_path = temp_dir / "customers.cdx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"DATABASE", "Sales", "", ""}, {"TABLE", "customers", "Sales", ""}});
    expect(dbc_create.ok, "SQL Server export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "CUST_ID", .type = 'N', .offset = 1U, .length = 6U, .decimal_count = 0U},
        {.name = "COMPANY", .type = 'C', .offset = 7U, .length = 40U, .decimal_count = 0U},
        {.name = "ACTIVE", .type = 'L', .offset = 47U, .length = 1U, .decimal_count = 0U},
        {.name = "BALANCE", .type = 'Y', .offset = 48U, .length = 8U, .decimal_count = 4U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"1001", "Acme Corp", "T", "123.45"}});
    expect(table_create.ok, "SQL Server export test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_cdx_bytes_for_postgresql_index_test();
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_sqlserver_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlserver_sql should resolve the customers fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("EXPORT DATABASE ... TYPE SQLSERVER") != std::string::npos,
           "export_database_as_sqlserver_sql should label its own header comment as a SQL Server export");
    expect(result.sql.find("CREATE TABLE [customers]") != std::string::npos,
           "export_database_as_sqlserver_sql should quote identifiers with square brackets, matching real T-SQL");
    expect(result.sql.find("[CUST_ID] DECIMAL(6, 0)") != std::string::npos,
           "export_database_as_sqlserver_sql should map a numeric field to DECIMAL, a real T-SQL type");
    expect(result.sql.find("[COMPANY] VARCHAR(40)") != std::string::npos,
           "export_database_as_sqlserver_sql should map a character field to VARCHAR(length)");
    expect(result.sql.find("[ACTIVE] BIT") != std::string::npos,
           "export_database_as_sqlserver_sql should map a logical field to BIT, since T-SQL has no BOOLEAN type");
    expect(result.sql.find("[BALANCE] MONEY") != std::string::npos,
           "export_database_as_sqlserver_sql should map VFP currency to MONEY, an exact match for its own 4-decimal-digit scale");
    expect(result.sql.find("INSERT INTO [customers]") != std::string::npos,
           "export_database_as_sqlserver_sql should emit an INSERT for the table's row");
    expect(result.sql.find("'Acme Corp'") != std::string::npos,
           "export_database_as_sqlserver_sql should quote a character value as a standard SQL string literal");
    expect(result.sql.find("(1001, 'Acme Corp', 1, 123.45)") != std::string::npos ||
               result.sql.find(", 1, 123.4500)") != std::string::npos,
           "export_database_as_sqlserver_sql should emit 1 (not the TRUE keyword) for a true logical value");
    expect(result.sql.find("TRUE") == std::string::npos && result.sql.find("FALSE") == std::string::npos,
           "export_database_as_sqlserver_sql must never emit the TRUE/FALSE keywords, which real T-SQL rejects in a VALUES list");

    expect(result.sql.find("CREATE INDEX [customers_CUST_ID_idx] ON [customers] ([CUST_ID]);") != std::string::npos,
           "export_database_as_sqlserver_sql should emit a CREATE INDEX for a tag whose key expression is a plain column reference");
    expect(result.sql.find("-- skipped index") != std::string::npos &&
               result.sql.find("COMPANY_N") != std::string::npos,
           "export_database_as_sqlserver_sql should report a composite-expression tag as a skipped index, not silently drop or mistranslate it");
    expect(result.sql.find("UPPER(company_name)") == std::string::npos,
           "export_database_as_sqlserver_sql must never emit a raw VFP key expression as if it were valid T-SQL syntax");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_sqlserver_sql_omits_indexes_without_cdx() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlserver_sql_no_cdx_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "SQL Server no-CDX export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "SKU", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"W-1"}});
    expect(table_create.ok, "SQL Server no-CDX export test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_sqlserver_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlserver_sql should succeed without a companion CDX: " + result.error);
    expect(result.sql.find("CREATE TABLE [widgets]") != std::string::npos,
           "export_database_as_sqlserver_sql should still emit the table without any index information");
    expect(result.sql.find("CREATE INDEX") == std::string::npos,
           "export_database_as_sqlserver_sql should not emit any CREATE INDEX when no companion CDX exists");

    fs::remove_all(temp_dir, ignored);
}

// Unlike PostgreSQL and SQLite (#5559, #5558), a real SQL Server 2022
// engine directly confirmed during this issue's own development that
// T-SQL index names are scoped *per table*, not schema-wide: two
// different tables can each carry an index of the identical name with
// no error. This is a positive test locking in that real, verified
// divergence -- table "A_B"'s tag "CDEF" and table "A"'s tag "B_CDEF"
// would collide under #5559's cross-table disambiguation on the other
// two dialects, but on SQL Server both must emit the exact same
// "A_B_CDEF_idx" name unchanged, since each is scoped to its own table.
void test_export_database_as_sqlserver_sql_allows_identical_index_name_across_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlserver_sql_same_name_across_tables_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "A_B", ""}, {"TABLE", "A", ""}});
    expect(dbc_create.ok, "SQL Server same-name-across-tables test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table1_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A_B.dbf").string(), table_fields, {{"x"}});
    expect(table1_create.ok, "SQL Server same-name-across-tables test: A_B.dbf fixture should be created");
    const auto table2_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A.dbf").string(), table_fields, {{"y"}});
    expect(table2_create.ok, "SQL Server same-name-across-tables test: A.dbf fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("CDEF", "COL");
        std::ofstream output(temp_dir / "A_B.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }
    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("B_CDEF", "COL");
        std::ofstream output(temp_dir / "A.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_sqlserver_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlserver_sql should resolve the same-name-across-tables fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("CREATE INDEX [A_B_CDEF_idx] ON [A_B] ([COL]);") != std::string::npos,
           "export_database_as_sqlserver_sql should emit the first table's own naturally-derived index name unchanged");
    expect(result.sql.find("CREATE INDEX [A_B_CDEF_idx] ON [A] ([COL]);") != std::string::npos,
           "export_database_as_sqlserver_sql should also emit the identical index name unchanged for the second table -- real SQL Server scopes index names per table, not schema-wide");
    expect(result.sql.find("CREATE INDEX [A_B_CDEF_idx_2]") == std::string::npos,
           "export_database_as_sqlserver_sql must not disambiguate a cross-table index name that real SQL Server does not actually collide on");

    fs::remove_all(temp_dir, ignored);
}

// Unlike PostgreSQL's silent truncation, a real SQL Server 2022 engine
// directly confirmed during this issue's own development that it
// *rejects* (error 103) any identifier over 128 characters outright --
// so this exporter must never construct one. A 125-character table name
// with two tags ("TAG1"/"TAG2") on the same column produces raw
// candidates that are identical for their first 128 bytes (index 128 is
// where "TAG1"/"TAG2" first differ), exercising the one collision that
// *can* still occur under SQL Server's real per-table scoping:
// truncation erasing the tag-derived suffix entirely when the table
// name alone is already at or beyond the limit.
void test_export_database_as_sqlserver_sql_disambiguates_indexes_within_identifier_length_limit() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlserver_sql_identifier_length_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::string long_table_name(125U, 'A');
    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / (long_table_name + ".dbf");
    const fs::path cdx_path = temp_dir / (long_table_name + ".cdx");
    // OBJECTNAME must be wide enough to hold the 125-byte long_table_name
    // itself (unlike the postgres/sqlite 63-byte-limit sibling tests,
    // whose 60-byte table name fit the usual 64-byte OBJECTNAME field).
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 150U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 167U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", long_table_name, ""}});
    expect(dbc_create.ok, "SQL Server identifier-length test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x"}});
    expect(table_create.ok, "SQL Server identifier-length test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_two_tag_cdx_bytes("TAG1", "TAG2", "COL");
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_sqlserver_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlserver_sql should resolve the identifier-length fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    // First candidate truncates to exactly 128 bytes: the 125 'A's plus
    // "_TA" (the two characters after the underscore "TAG1"/"TAG2" still
    // share before diverging at what would be index 128).
    const std::string first_truncated = long_table_name + "_TA";
    expect(first_truncated.size() == 128U,
           "test fixture sanity: the first truncated candidate should be exactly 128 bytes");
    expect(result.sql.find("CREATE INDEX [" + first_truncated + "]") != std::string::npos,
           "export_database_as_sqlserver_sql should truncate the first candidate to SQL Server's 128-byte identifier limit");

    const std::string second_suffixed = long_table_name + "__2";
    expect(second_suffixed.size() == 128U,
           "test fixture sanity: the disambiguated second candidate should still be exactly 128 bytes");
    expect(result.sql.find("CREATE INDEX [" + second_suffixed + "]") != std::string::npos,
           "export_database_as_sqlserver_sql should reserve room for the disambiguating suffix within the 128-byte limit");
    expect(result.sql.find("CREATE INDEX [" + first_truncated + "]", result.sql.find("CREATE INDEX [" + first_truncated + "]") + 1U)
               == std::string::npos,
           "export_database_as_sqlserver_sql must never emit the exact same truncated index name twice on the same table");

    fs::remove_all(temp_dir, ignored);
}

// #5554 PR review (chatgpt-codex-connector, P1): a blank VFP date field
// decodes to an empty display_value (not is_null), and real SQL Server
// silently converts an empty-string literal to 1900-01-01 rather than
// erroring -- directly confirmed against a real local SQL Server 2022
// engine (`CAST('' AS DATE)` returns 1900-01-01, not an error) -- so
// emitting `''` for a blank date silently invents data instead of
// preserving the blank. A non-blank date must still emit as a normal
// quoted "YYYY-MM-DD" string, unaffected by this fix.
void test_export_database_as_sqlserver_sql_preserves_blank_dates_as_null() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlserver_sql_blank_date_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "events.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "events", ""}});
    expect(dbc_create.ok, "SQL Server blank-date test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "OCCURRED", .type = 'D', .offset = 1U, .length = 8U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"20260115"}, {""}});
    expect(table_create.ok, "SQL Server blank-date test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_sqlserver_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlserver_sql should resolve the blank-date fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("VALUES ('2026-01-15')") != std::string::npos,
           "export_database_as_sqlserver_sql should emit a non-blank date as a normal quoted string literal");
    expect(result.sql.find("VALUES (NULL)") != std::string::npos,
           "export_database_as_sqlserver_sql should emit NULL for a blank date rather than an empty string literal");
    expect(result.sql.find("VALUES ('')") == std::string::npos,
           "export_database_as_sqlserver_sql must never emit an empty string literal for a DATE column -- real SQL Server silently converts it to 1900-01-01 instead of erroring");

    fs::remove_all(temp_dir, ignored);
}

// #5554 PR review (chatgpt-codex-connector and copilot-pull-request-reviewer,
// independently): length/decimal_count are raw uint8_t values from a DBF
// header this codebase's own writer never produces out of range for, but
// a crafted or foreign source is not bound by that. Real SQL Server
// rejects any DECIMAL precision above 38 or scale above its own
// precision (directly confirmed against a real local SQL Server 2022
// engine: DECIMAL(39, 0) fails, DECIMAL(38, 38) succeeds), so an
// unclamped DECIMAL(255, 255) would make this exporter's own advertised
// "loads without error" promise false for exactly the malformed input a
// real-world corrupt table is most likely to contain.
void test_export_database_as_sqlserver_sql_clamps_decimal_precision_and_scale() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlserver_sql_decimal_clamp_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "amounts.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "amounts", ""}});
    expect(dbc_create.ok, "SQL Server DECIMAL-clamp test: DBC fixture should be created");

    // A genuinely valid DBF numeric field can never carry length/decimal_count
    // this large (VFP's own N field caps at 20 digits) -- construct the
    // descriptor directly (bypassing create_dbf_table_file's own writer,
    // which would refuse this) to model a crafted/corrupt header the way
    // the review comment describes.
    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "AMOUNT", .type = 'N', .offset = 1U, .length = 255U, .decimal_count = 255U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"0"}});
    expect(table_create.ok, "SQL Server DECIMAL-clamp test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_sqlserver_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlserver_sql should resolve the DECIMAL-clamp fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("[AMOUNT] DECIMAL(38, 38)") != std::string::npos,
           "export_database_as_sqlserver_sql should clamp precision to SQL Server's 38-digit maximum, and scale to that same clamped precision");
    expect(result.sql.find("DECIMAL(255") == std::string::npos,
           "export_database_as_sqlserver_sql must never emit a DECIMAL precision real SQL Server rejects");

    fs::remove_all(temp_dir, ignored);
}

// #5554 PR review (chatgpt-codex-connector, P2): SQL Server's 128-limit
// on `sysname` (`NVARCHAR(128)`) is a *character* count, not a UTF-8
// *byte* count -- for a table name built from 2-byte UTF-8 characters
// ("\xC3\xA9", U+00E9 "e with acute"), a byte-count-based truncation to
// 128 bytes would keep only 64 characters, while the correct
// character-count truncation keeps 128. 120 2-byte characters (240
// bytes) is the widest fixture this test can use and still leave the
// physical .dbf/.cdx filenames -- which must exactly match the table
// name -- under the real filesystem's 255-byte NAME_MAX once
// write_binary_file()'s own ".cptmp"/".cpbak" atomic-write suffixes are
// appended (240 + strlen(".dbf.cptmp") == 250, safely under 255); a
// single-tag fixture (no cross-tag collision needed) keeps the
// assertion math simple while still conclusively distinguishing
// character-count from byte-count truncation.
void test_export_database_as_sqlserver_sql_truncates_by_unicode_character_not_byte_count() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_sqlserver_sql_utf8_length_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::string two_byte_char = "\xC3\xA9";  // U+00E9, 2 UTF-8 bytes, 1 code point
    std::string long_table_name;
    for (std::size_t i = 0U; i < 120U; ++i) {
        long_table_name += two_byte_char;
    }
    expect(long_table_name.size() == 240U,
           "test fixture sanity: 120 2-byte characters should be exactly 240 bytes");

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / (long_table_name + ".dbf");
    const fs::path cdx_path = temp_dir / (long_table_name + ".cdx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 244U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 261U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", long_table_name, ""}});
    expect(dbc_create.ok, "SQL Server UTF-8 identifier-length test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x"}});
    expect(table_create.ok, "SQL Server UTF-8 identifier-length test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("TAG1", "COL");
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_sqlserver_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_sqlserver_sql should resolve the UTF-8 identifier-length fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    // Full candidate is 120 (table name) + 9 ("_TAG1_idx") = 129 code
    // points. Truncating to 128 code points keeps all 120 of the table
    // name's own characters plus "_TAG1_id" (the first 8 of the 9
    // trailing ASCII characters).
    const std::string correctly_truncated = long_table_name + "_TAG1_id";
    expect(result.sql.find("CREATE INDEX [" + correctly_truncated + "]") != std::string::npos,
           "export_database_as_sqlserver_sql should truncate by 128 Unicode code points, keeping every one of the table name's own 120 2-byte characters");

    // A byte-count-based truncation (the pre-fix behavior) would instead
    // stop after only 64 of the 2-byte characters, with none of the
    // ASCII suffix surviving at all.
    std::string wrong_byte_truncated;
    for (std::size_t i = 0U; i < 64U; ++i) {
        wrong_byte_truncated += two_byte_char;
    }
    expect(result.sql.find("CREATE INDEX [" + wrong_byte_truncated + "]") == std::string::npos,
           "export_database_as_sqlserver_sql must not fall back to counting bytes instead of Unicode code points");

    fs::remove_all(temp_dir, ignored);
}

// #5554 PR review (chatgpt-codex-connector): the same underlying defect
// fixed for SQL Server's character-count truncation also applied to
// PostgreSQL's own byte-count truncation (#5559) -- a plain
// `substr(0, 63)` byte cut can land inside a multi-byte UTF-8 sequence,
// emitting malformed UTF-8. utf8_safe_truncate()'s byte-mode backs off
// to the last complete character boundary instead. A table name of 40
// 2-byte characters ("\xC3\xA9") plus a tag suffix produces a candidate
// whose naive 63-byte cut would land mid-character (byte 63 is the
// second, continuation byte of the 32nd character) -- the fix must back
// off to 62 bytes (31 complete characters) rather than emit the
// 63-byte prefix ending in a lone, unpaired lead byte.
void test_export_database_as_postgresql_sql_truncates_on_utf8_character_boundary() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_postgresql_sql_utf8_boundary_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::string two_byte_char = "\xC3\xA9";  // U+00E9, 2 UTF-8 bytes, 1 code point
    std::string table_name;
    for (std::size_t i = 0U; i < 40U; ++i) {
        table_name += two_byte_char;
    }
    expect(table_name.size() == 80U,
           "test fixture sanity: 40 2-byte characters should be exactly 80 bytes");

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / (table_name + ".dbf");
    const fs::path cdx_path = temp_dir / (table_name + ".cdx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 100U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 117U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", table_name, ""}});
    expect(dbc_create.ok, "PostgreSQL UTF-8 boundary test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x"}});
    expect(table_create.ok, "PostgreSQL UTF-8 boundary test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("CDEF", "COL");
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_postgresql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_postgresql_sql should resolve the UTF-8 boundary fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    std::string correctly_truncated;
    for (std::size_t i = 0U; i < 31U; ++i) {
        correctly_truncated += two_byte_char;
    }
    expect(correctly_truncated.size() == 62U,
           "test fixture sanity: 31 complete 2-byte characters should be exactly 62 bytes");
    expect(result.sql.find("CREATE INDEX \"" + correctly_truncated + "\"") != std::string::npos,
           "export_database_as_postgresql_sql should back off to the last complete UTF-8 character when a 63-byte cut would split one");

    const std::string malformed_63_byte_cut = table_name.substr(0U, 63U);
    expect(result.sql.find("CREATE INDEX \"" + malformed_63_byte_cut + "\"") == std::string::npos,
           "export_database_as_postgresql_sql must never emit an index name ending in an incomplete multi-byte UTF-8 sequence");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_oracle_sql_maps_types_and_creates_indexes() {
    // #5554 (parent #137, fourth vendor-dialect slice, following #5537's
    // PostgreSQL, #5558's SQLite, and #5561's SQL Server precedent):
    // real Oracle 23ai (verified against a local container during this
    // issue's own development) directly confirmed to accept this exact
    // dialect -- double-quoted identifiers, single-quoted string
    // literals, NUMBER/VARCHAR2/NUMBER(1)/DATE column types, and the
    // ANSI DATE 'YYYY-MM-DD' literal form -- loading a representative
    // CREATE TABLE/INSERT/CREATE INDEX script shaped exactly like this
    // exporter's own output without error, including a cross-table JOIN
    // returning the correct row. This test proves the same genuinely
    // new piece #5537's own sibling test proves (CREATE INDEX
    // statements derived from a table's production CDX tags, correctly
    // distinguishing a plain column reference from a composite
    // expression), plus Oracle's own two real dialect requirements: a
    // NUMBER(1) column's literal is 1/0 (directly confirmed against the
    // real engine: unlike T-SQL, Oracle 23c's own TRUE *does* implicitly
    // convert for a NUMBER(1) column, but this exporter deliberately
    // targets 1/0 for compatibility with every Oracle version, not just
    // the newest), and a DATE value must use the ANSI DATE '...'
    // literal form, not a bare string (directly confirmed: a bare
    // 'YYYY-MM-DD' string fails with ORA-01861 against the real
    // engine's own default NLS_DATE_FORMAT).
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "customers.dbf";
    const fs::path cdx_path = temp_dir / "customers.cdx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"DATABASE", "Sales", "", ""}, {"TABLE", "customers", "Sales", ""}});
    expect(dbc_create.ok, "Oracle export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "CUST_ID", .type = 'N', .offset = 1U, .length = 6U, .decimal_count = 0U},
        {.name = "COMPANY", .type = 'C', .offset = 7U, .length = 40U, .decimal_count = 0U},
        {.name = "ACTIVE", .type = 'L', .offset = 47U, .length = 1U, .decimal_count = 0U},
        {.name = "BALANCE", .type = 'Y', .offset = 48U, .length = 8U, .decimal_count = 4U},
        {.name = "SIGNUP", .type = 'D', .offset = 56U, .length = 8U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"1001", "Acme Corp", "T", "123.45", "20260115"}});
    expect(table_create.ok, "Oracle export test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_cdx_bytes_for_postgresql_index_test();
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should resolve the customers fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("EXPORT DATABASE ... TYPE ORACLE") != std::string::npos,
           "export_database_as_oracle_sql should label its own header comment as an Oracle export");
    expect(result.sql.find("CREATE TABLE \"customers\"") != std::string::npos,
           "export_database_as_oracle_sql should quote identifiers with double quotes, matching real Oracle");
    expect(result.sql.find("\"CUST_ID\" NUMBER(6, 0)") != std::string::npos,
           "export_database_as_oracle_sql should map a numeric field to NUMBER, a real Oracle type");
    expect(result.sql.find("\"COMPANY\" VARCHAR2(40 CHAR)") != std::string::npos,
           "export_database_as_oracle_sql should map a character field to VARCHAR2(length CHAR), not the reserved/deprecated VARCHAR nor a bare byte-counted VARCHAR2(length)");
    expect(result.sql.find("\"ACTIVE\" NUMBER(1)") != std::string::npos,
           "export_database_as_oracle_sql should map a logical field to NUMBER(1), since Oracle has no dedicated table-column BOOLEAN type");
    expect(result.sql.find("\"BALANCE\" NUMBER(19, 4)") != std::string::npos,
           "export_database_as_oracle_sql should map VFP currency to NUMBER(19, 4), an exact match for its own 4-decimal-digit scale");
    expect(result.sql.find("\"SIGNUP\" DATE") != std::string::npos,
           "export_database_as_oracle_sql should map a date field to DATE, a real Oracle type");
    expect(result.sql.find("INSERT INTO \"customers\"") != std::string::npos,
           "export_database_as_oracle_sql should emit an INSERT for the table's row");
    expect(result.sql.find("'Acme Corp'") != std::string::npos,
           "export_database_as_oracle_sql should quote a character value as a standard SQL string literal");
    expect(result.sql.find("DATE '2026-01-15'") != std::string::npos,
           "export_database_as_oracle_sql should wrap a non-blank date in Oracle's own ANSI DATE literal syntax, not a bare string");
    expect(result.sql.find(", 1, 123.4500, DATE") != std::string::npos,
           "export_database_as_oracle_sql should emit 1 (not the TRUE keyword) for a true logical value");
    expect(result.sql.find("TRUE") == std::string::npos && result.sql.find("FALSE") == std::string::npos,
           "export_database_as_oracle_sql should not emit the TRUE/FALSE keywords, targeting every Oracle version rather than only 23c's own new boolean-literal support");

    expect(result.sql.find("CREATE INDEX \"customers_CUST_ID_idx\" ON \"customers\" (\"CUST_ID\");") != std::string::npos,
           "export_database_as_oracle_sql should emit a CREATE INDEX for a tag whose key expression is a plain column reference");
    expect(result.sql.find("-- skipped index") != std::string::npos &&
               result.sql.find("COMPANY_N") != std::string::npos,
           "export_database_as_oracle_sql should report a composite-expression tag as a skipped index, not silently drop or mistranslate it");
    expect(result.sql.find("UPPER(company_name)") == std::string::npos,
           "export_database_as_oracle_sql must never emit a raw VFP key expression as if it were valid Oracle syntax");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_oracle_sql_omits_indexes_without_cdx() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_no_cdx_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "Oracle no-CDX export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "SKU", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"W-1"}});
    expect(table_create.ok, "Oracle no-CDX export test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should succeed without a companion CDX: " + result.error);
    expect(result.sql.find("CREATE TABLE \"widgets\"") != std::string::npos,
           "export_database_as_oracle_sql should still emit the table without any index information");
    expect(result.sql.find("CREATE INDEX") == std::string::npos,
           "export_database_as_oracle_sql should not emit any CREATE INDEX when no companion CDX exists");

    fs::remove_all(temp_dir, ignored);
}

// Unlike SQL Server (#5554) but like PostgreSQL/SQLite (#5559, #5558),
// a real Oracle 23ai engine directly confirmed during this issue's own
// development that index names must be unique *schema-wide*: two
// different tables cannot each carry an index of the identical name
// (ORA-00955, "name is already used by an existing object"). Mirrors
// the identical PostgreSQL/SQLite cross-table collision fixtures
// exactly.
void test_export_database_as_oracle_sql_disambiguates_indexes_across_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_cross_table_collision_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "A_B", ""}, {"TABLE", "A", ""}});
    expect(dbc_create.ok, "Oracle cross-table index-collision test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table1_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A_B.dbf").string(), table_fields, {{"x"}});
    expect(table1_create.ok, "Oracle cross-table index-collision test: A_B.dbf fixture should be created");
    const auto table2_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A.dbf").string(), table_fields, {{"y"}});
    expect(table2_create.ok, "Oracle cross-table index-collision test: A.dbf fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("CDEF", "COL");
        std::ofstream output(temp_dir / "A_B.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }
    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("B_CDEF", "COL");
        std::ofstream output(temp_dir / "A.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should resolve the cross-table collision fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    const std::size_t first_occurrence = result.sql.find("CREATE INDEX \"A_B_CDEF_idx\"");
    expect(first_occurrence != std::string::npos,
           "export_database_as_oracle_sql should emit the first table's own naturally-derived index name unchanged");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx\"", first_occurrence + 1U) == std::string::npos,
           "export_database_as_oracle_sql must never emit the exact same index name twice across different tables");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx_2\"") != std::string::npos,
           "export_database_as_oracle_sql should disambiguate the second table's colliding index name with a deterministic suffix");

    fs::remove_all(temp_dir, ignored);
}

// Unlike PostgreSQL (#5559) but like SQL Server (#5554), a real Oracle
// 23ai engine directly confirmed during this issue's own development
// that tables and indexes live in *separate* namespaces: a table can
// share its own name with an unrelated table's index with no collision
// at all. This is a positive test locking in that real, verified
// divergence -- a table literally named "A_B_CDEF_idx" must not force
// table "A_B"'s own naturally-derived "A_B_CDEF_idx" index name to be
// disambiguated, unlike PostgreSQL's own #5559 fix for the identical-
// looking scenario.
void test_export_database_as_oracle_sql_allows_index_named_after_a_table() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_table_index_namespace_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "A_B", ""}, {"TABLE", "A_B_CDEF_idx", ""}});
    expect(dbc_create.ok, "Oracle table/index namespace test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table1_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A_B.dbf").string(), table_fields, {{"x"}});
    expect(table1_create.ok, "Oracle table/index namespace test: A_B.dbf fixture should be created");
    const auto table2_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A_B_CDEF_idx.dbf").string(), table_fields, {{"y"}});
    expect(table2_create.ok, "Oracle table/index namespace test: A_B_CDEF_idx.dbf fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("CDEF", "COL");
        std::ofstream output(temp_dir / "A_B.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should resolve the table/index namespace fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("CREATE TABLE \"A_B_CDEF_idx\"") != std::string::npos,
           "export_database_as_oracle_sql should still emit the oddly-named table itself");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx\"") != std::string::npos,
           "export_database_as_oracle_sql should emit the index name unchanged -- Oracle keeps tables and indexes in separate namespaces");
    expect(result.sql.find("CREATE INDEX \"A_B_CDEF_idx_2\"") == std::string::npos,
           "export_database_as_oracle_sql must not disambiguate an index name that does not actually collide with a table name on this engine");

    fs::remove_all(temp_dir, ignored);
}

// A real Oracle 23ai engine directly confirmed during this issue's own
// development that it *rejects* (ORA-00972, "identifier ... exceeds the
// maximum length of 128 bytes") any identifier over 128 *bytes*
// outright -- a byte limit like PostgreSQL's own 63-byte one (#5559),
// not a character limit like SQL Server's (#5554). Mirrors
// test_export_database_as_postgresql_sql_disambiguates_indexes_within_identifier_length_limit()'s
// own fixture shape exactly, just scaled to 128 bytes.
void test_export_database_as_oracle_sql_disambiguates_indexes_within_identifier_length_limit() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_identifier_length_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::string long_table_name(125U, 'A');
    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / (long_table_name + ".dbf");
    const fs::path cdx_path = temp_dir / (long_table_name + ".cdx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 150U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 167U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", long_table_name, ""}});
    expect(dbc_create.ok, "Oracle identifier-length test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x"}});
    expect(table_create.ok, "Oracle identifier-length test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_two_tag_cdx_bytes("TAG1", "TAG2", "COL");
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should resolve the identifier-length fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    // 128 bytes: all 125 of the table name's own characters, plus "_TA"
    // (the two ASCII characters after the underscore "TAG1"/"TAG2"
    // still share before diverging at what would be byte 128).
    const std::string first_truncated = long_table_name + "_TA";
    expect(first_truncated.size() == 128U,
           "test fixture sanity: the first truncated candidate should be exactly 128 bytes");
    expect(result.sql.find("CREATE INDEX \"" + first_truncated + "\"") != std::string::npos,
           "export_database_as_oracle_sql should truncate the first candidate to Oracle's 128-byte identifier limit");

    const std::string second_suffixed = long_table_name + "__2";
    expect(second_suffixed.size() == 128U,
           "test fixture sanity: the disambiguated second candidate should still be exactly 128 bytes");
    expect(result.sql.find("CREATE INDEX \"" + second_suffixed + "\"") != std::string::npos,
           "export_database_as_oracle_sql should reserve room for the disambiguating suffix within the 128-byte limit");
    expect(result.sql.find("CREATE INDEX \"" + first_truncated + "\"", result.sql.find("CREATE INDEX \"" + first_truncated + "\"") + 1U)
               == std::string::npos,
           "export_database_as_oracle_sql must never emit the exact same truncated index name twice");

    fs::remove_all(temp_dir, ignored);
}

// #5554 PR review pattern applied proactively (matching #5562's own
// blank-date fix for SQL Server): a blank VFP date field decodes to an
// empty display_value (not is_null). Unlike SQL Server, where the
// empty-string form merely silently invents 1900-01-01, Oracle's own
// `DATE ''` is invalid syntax outright (ORA-01841, directly confirmed
// against a real local Oracle 23ai engine) -- so this is a basic
// correctness requirement here, not merely a data-integrity one. A
// non-blank date must still wrap in the ANSI DATE '...' literal form.
void test_export_database_as_oracle_sql_preserves_blank_dates_as_null() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_blank_date_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "events.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "events", ""}});
    expect(dbc_create.ok, "Oracle blank-date test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "OCCURRED", .type = 'D', .offset = 1U, .length = 8U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"20260115"}, {""}});
    expect(table_create.ok, "Oracle blank-date test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should resolve the blank-date fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("VALUES (DATE '2026-01-15')") != std::string::npos,
           "export_database_as_oracle_sql should wrap a non-blank date in the ANSI DATE literal form");
    expect(result.sql.find("VALUES (NULL)") != std::string::npos,
           "export_database_as_oracle_sql should emit NULL for a blank date rather than an invalid DATE '' literal");
    expect(result.sql.find("DATE ''") == std::string::npos,
           "export_database_as_oracle_sql must never emit DATE '' -- real Oracle rejects it outright (ORA-01841), unlike SQL Server's own silently-wrong empty-string case");

    fs::remove_all(temp_dir, ignored);
}

// #5554 PR review pattern applied proactively (matching #5562's own
// DECIMAL clamp fix for SQL Server): length/decimal_count are raw
// uint8_t values from a DBF header this codebase's own writer never
// produces out of range for, but a crafted or foreign source is not
// bound by that. Real Oracle rejects any NUMBER precision above 38
// (directly confirmed: NUMBER(38, 0) succeeds, NUMBER(39, 0) fails,
// "numeric precision specifier is out of range (1 to 38)") and any
// scale above 127 (NUMBER(10, 127) succeeds, NUMBER(10, 128) fails,
// "numeric scale specifier is out of range (-84 to 127)") -- but,
// unlike SQL Server's own DECIMAL, scale is *not* clamped to precision:
// a scale far exceeding precision is directly confirmed valid on the
// real engine (NUMBER(10, 20) succeeds), so this test specifically
// proves scale is clamped to 127 alone, not to the (much smaller)
// clamped precision the way sqlserver_column_type()'s own clamp works.
void test_export_database_as_oracle_sql_clamps_number_precision_and_scale() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_number_clamp_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "amounts.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "amounts", ""}});
    expect(dbc_create.ok, "Oracle NUMBER-clamp test: DBC fixture should be created");

    // A genuinely valid DBF numeric field can never carry length/decimal_count
    // this large (VFP's own N field caps at 20 digits) -- construct the
    // descriptor directly (bypassing create_dbf_table_file's own writer,
    // which would refuse this) to model a crafted/corrupt header.
    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "AMOUNT", .type = 'N', .offset = 1U, .length = 255U, .decimal_count = 255U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"0"}});
    expect(table_create.ok, "Oracle NUMBER-clamp test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should resolve the NUMBER-clamp fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("\"AMOUNT\" NUMBER(38, 127)") != std::string::npos,
           "export_database_as_oracle_sql should clamp precision to Oracle's 38-digit maximum and scale to Oracle's own independent 127 maximum, not to the clamped precision");
    expect(result.sql.find("NUMBER(255") == std::string::npos && result.sql.find(", 255)") == std::string::npos,
           "export_database_as_oracle_sql must never emit a NUMBER precision or scale real Oracle rejects");

    fs::remove_all(temp_dir, ignored);
}

// #5554 review pattern applied proactively: unlike every other dialect
// this file emits, Oracle provides no escape mechanism for an embedded
// double quote inside a quoted identifier at all -- directly confirmed
// against a real local Oracle 23ai engine that
// `CREATE TABLE "weird""name" (...)` fails outright with ORA-25716
// ("The identifier contains a double quotation mark (\") character"),
// not the doubled-quote-survives-as-literal-quote behavior every other
// dialect's own doubling convention relies on.
void test_export_database_as_oracle_sql_strips_embedded_quote_from_identifiers() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_embedded_quote_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "Oracle embedded-quote test: DBC fixture should be created");

    // A genuinely valid VFP field name can never contain a literal `"`
    // -- construct the descriptor directly to model a crafted/corrupt
    // source, matching this codebase's own established practice for
    // exercising fail-closed/lossy-but-safe handling of input a real
    // writer never produces.
    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "weird\"name", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x"}});
    expect(table_create.ok, "Oracle embedded-quote test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should resolve the embedded-quote fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("\"weirdname\"") != std::string::npos,
           "export_database_as_oracle_sql should strip an embedded double quote from an identifier, since real Oracle has no escape mechanism for one at all");
    expect(result.sql.find("weird\"\"name") == std::string::npos,
           "export_database_as_oracle_sql must never emit a doubled-quote escape for an identifier -- real Oracle rejects any embedded quote outright, doubled or not");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_oracle_sql_declares_character_columns_with_char_semantics() {
    // #5564 PR review (chatgpt-codex-connector, P1): a bare VARCHAR2(n) is
    // measured in *bytes* under Oracle's own default
    // NLS_LENGTH_SEMANTICS=BYTE -- directly confirmed against a real local
    // Oracle 23ai engine -- so a VFP C(n) field's own character count must
    // be declared with explicit CHAR semantics to guarantee n characters
    // of capacity regardless of the target session's own NLS settings.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_varchar2_char_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "Oracle VARCHAR2 CHAR-semantics test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"widget-a"}});
    expect(table_create.ok, "Oracle VARCHAR2 CHAR-semantics test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should resolve the VARCHAR2 CHAR-semantics fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("\"NAME\" VARCHAR2(10 CHAR)") != std::string::npos,
           "export_database_as_oracle_sql should declare a C(n) column as VARCHAR2(n CHAR), since Oracle's own default byte-counted semantics would otherwise reject a real multi-byte UTF-8 value that fits in n characters");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_oracle_sql_writes_memo_content_as_clob_literals() {
    // #5564 PR review (chatgpt-codex-connector, P1): a CLOB column's own
    // literal cannot be a plain quoted string -- directly confirmed
    // against a real local Oracle 23ai engine that an empty string
    // literal silently becomes NULL for a CLOB column, and that Oracle's
    // own SQL text-literal limit is 4000 *bytes*. Exercises all three of
    // oracle_clob_literal()'s own code paths (empty -> EMPTY_CLOB(),
    // short non-empty -> a single TO_CLOB(...), and content over the
    // 4000-byte chunk size -> TO_CLOB(...) || TO_CLOB(...) concatenation)
    // through a real M-type memo field, since the function itself lives
    // in this file's own anonymous namespace and is not otherwise
    // directly unit-testable.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_clob_literal_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "Oracle CLOB literal test: DBC fixture should be created");

    const std::string short_note = "short note";
    const std::string large_note(4500U, 'A');
    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "NOTES", .type = 'M', .offset = 1U, .length = 4U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{""}, {short_note}, {large_note}});
    expect(table_create.ok, "Oracle CLOB literal test: DBF fixture with memo content should be created");

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_oracle_sql should resolve the CLOB literal fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("VALUES (EMPTY_CLOB())") != std::string::npos,
           "export_database_as_oracle_sql should emit EMPTY_CLOB() for a blank memo, since a plain '' literal silently becomes NULL for a real Oracle CLOB column");
    expect(result.sql.find("VALUES (TO_CLOB('" + short_note + "'))") != std::string::npos,
           "export_database_as_oracle_sql should emit a single TO_CLOB(...) for a memo well under the 4000-byte chunk size");
    const std::string first_chunk(4000U, 'A');
    const std::string second_chunk(500U, 'A');
    expect(result.sql.find(
               "TO_CLOB('" + first_chunk + "') || TO_CLOB('" + second_chunk + "')") != std::string::npos,
           "export_database_as_oracle_sql should split a memo over Oracle's own 4000-byte SQL text-literal limit into TO_CLOB(...) || TO_CLOB(...) concatenation");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_oracle_sql_fails_closed_on_colliding_identifiers() {
    // #5564 PR review (chatgpt-codex-connector, P2): oracle_quote_identifier()'s
    // own embedded-quote stripping is not collision-safe -- distinct source
    // names (e.g. "ab" and "a\"b") can sanitize to the identical quoted
    // identifier. export_database_as_oracle_sql() must fail the whole
    // export closed rather than emit a script with a duplicate/wrong-
    // target column.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_identifier_collision_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "Oracle identifier collision test: DBC fixture should be created");

    // A genuinely valid VFP field name can never contain a literal `"` --
    // construct the descriptors directly to model a crafted/corrupt
    // source, matching this file's own established practice (see
    // test_export_database_as_oracle_sql_strips_embedded_quote_from_identifiers()
    // above) for exercising fail-closed handling of input a real writer
    // never produces.
    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "ab", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
        {.name = "a\"b", .type = 'C', .offset = 11U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x", "y"}});
    expect(table_create.ok, "Oracle identifier collision test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(!result.ok,
           "export_database_as_oracle_sql should fail closed when two column names collide after quote-stripping, rather than emit a script with a duplicate/wrong-target column");
    expect(result.sql.empty(),
           "export_database_as_oracle_sql should not return a partial SQL script alongside a failure");
    expect(result.error.find("\"ab\"") != std::string::npos,
           "export_database_as_oracle_sql's collision error should name the colliding identifier: " + result.error);

    fs::remove_all(temp_dir, ignored);
}

// #5693: Oracle treats a zero-length VARCHAR2 literal as NULL -- directly
// confirmed against a real local Oracle 23ai engine (`INSERT INTO ...
// VALUES ('')` into a VARCHAR2(n CHAR) column leaves it NULL, `IS NULL`
// reports TRUE). A DBF row can genuinely hold a non-null empty Character
// value (decode_value()'s own 'C' case never reports is_null for a
// present-but-blank field, only a genuinely absent one), and there is no
// VARCHAR2 literal that preserves that value's own non-null-ness -- so
// export_database_as_oracle_sql() must fail the whole export closed
// rather than silently corrupt the distinction, matching this exporter's
// own precedent for blank Memo (EMPTY_CLOB()) and blank Date (NULL, since
// `DATE ''` is invalid syntax) each already established for their own
// unrepresentable-or-lossy cases.
void test_export_database_as_oracle_sql_fails_closed_on_non_null_empty_character_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_empty_character_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "Oracle empty-character test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{""}});
    expect(table_create.ok, "Oracle empty-character test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(!result.ok,
           "export_database_as_oracle_sql should fail closed on a non-null empty character value, since no VARCHAR2 literal preserves its non-null-ness against a real Oracle engine");
    expect(result.sql.empty(),
           "export_database_as_oracle_sql should not return a partial SQL script alongside a failure");
    expect(result.error.find("widgets") != std::string::npos && result.error.find("NAME") != std::string::npos,
           "export_database_as_oracle_sql's empty-character error should name the offending table and column: " + result.error);

    fs::remove_all(temp_dir, ignored);
}

// #5717 PR review (chatgpt-codex-connector, P1): this codebase does not
// currently decode a VFP nullable field's own `_NullFlags` record bitmap
// at all, so a genuinely null value and a genuinely non-null empty value
// in a *nullable* field are indistinguishable in this codebase's own
// in-memory representation. The empty-character-value check above must
// not apply to a table that declares a nullable field (identified here by
// the presence of the special type-`0` `_NullFlags` pseudo-field in its
// own descriptor list), since it cannot tell the two cases apart and
// would otherwise reject a genuinely null value's own (previously
// correctly working, if by Oracle's own accidental empty-string-is-null
// behavior) export. create_dbf_table_file() itself cannot write a type-`0`
// field (it is not among the directly-writable storage types), so this
// fixture is built from raw DBF bytes directly, modeling a real VFP
// nullable-field table this codebase's own writer cannot itself produce
// but a real VFP application routinely can.
void test_export_database_as_oracle_sql_allows_blank_value_in_table_with_nullable_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_oracle_sql_nullable_field_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "Oracle nullable-field test: DBC fixture should be created");

    // Raw VFP-style DBF bytes: one Character field ("NAME", blank) and one
    // type-'0' _NullFlags pseudo-field (1 byte, content irrelevant to this
    // test -- only its mere presence in the descriptor list matters).
    constexpr std::uint16_t record_length = 1U + 10U + 1U;  // delete flag + NAME(10) + _NullFlags(1)
    constexpr std::uint16_t header_length = 32U + 32U + 32U + 1U;  // header + 2 field descriptors + terminator
    std::vector<std::uint8_t> bytes(
        static_cast<std::size_t>(header_length) + record_length + 1U, 0U);
    bytes[0] = 0x30U;  // VFP version byte
    bytes[4] = 1U;     // record count (LE u32, 1 record)
    bytes[8] = static_cast<std::uint8_t>(header_length & 0xFFU);
    bytes[9] = static_cast<std::uint8_t>((header_length >> 8U) & 0xFFU);
    bytes[10] = static_cast<std::uint8_t>(record_length & 0xFFU);
    bytes[11] = static_cast<std::uint8_t>((record_length >> 8U) & 0xFFU);

    const auto write_descriptor = [&](std::size_t offset, const char* name, char type,
                                       std::uint32_t field_offset, std::uint8_t length) {
        std::memcpy(bytes.data() + offset, name, std::strlen(name));
        bytes[offset + 11U] = static_cast<std::uint8_t>(type);
        bytes[offset + 12U] = static_cast<std::uint8_t>(field_offset & 0xFFU);
        bytes[offset + 13U] = static_cast<std::uint8_t>((field_offset >> 8U) & 0xFFU);
        bytes[offset + 14U] = static_cast<std::uint8_t>((field_offset >> 16U) & 0xFFU);
        bytes[offset + 15U] = static_cast<std::uint8_t>((field_offset >> 24U) & 0xFFU);
        bytes[offset + 16U] = length;
    };
    write_descriptor(32U, "NAME", 'C', 1U, 10U);
    write_descriptor(64U, "_NullFlags", '0', 11U, 1U);
    bytes[96] = 0x0DU;  // field descriptor terminator

    const std::size_t record_offset = header_length;
    bytes[record_offset] = 0x20U;  // not deleted
    std::memset(bytes.data() + record_offset + 1U, ' ', 10U);  // blank NAME
    bytes[record_offset + 11U] = 0U;  // _NullFlags byte, content irrelevant here
    bytes.back() = 0x1AU;  // EOF marker

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_oracle_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok,
           "export_database_as_oracle_sql should not fail closed on a blank character value in a table that declares a nullable field, since it cannot currently tell a genuine NULL from a genuine empty string there: " + result.error);
    if (result.ok) {
        expect(result.sql.find("VALUES ('', NULL)") != std::string::npos,
               "export_database_as_oracle_sql should still emit a plain '' literal for the blank NAME value in a nullable-field table (the pre-existing, Oracle-accidental-NULL behavior), not reject the export: " + result.sql);
    }

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_mysql_sql_maps_types_and_creates_indexes() {
    // #5554 (parent #137, fifth and final vendor-dialect slice, following
    // #5537's PostgreSQL, #5558's SQLite, #5561's SQL Server, and #5564's
    // Oracle precedent): real MySQL 8.0 (verified against a local
    // container during this issue's own development) directly confirmed
    // to accept this exact dialect -- backtick identifiers, single-quoted
    // string literals, and DECIMAL/INT/DOUBLE/TINYINT(1)/DATE/DATETIME/
    // VARCHAR(length)/LONGTEXT column types -- loading a representative
    // CREATE TABLE/INSERT/CREATE INDEX script shaped exactly like this
    // exporter's own output without error. This test proves the same
    // genuinely new piece #5537's own sibling test proves (CREATE INDEX
    // statements derived from a table's production CDX tags, correctly
    // distinguishing a plain column reference from a composite
    // expression), plus the TINYINT(1) column's 1/0 literal, matching
    // this file's own established cross-version-safe convention (real
    // MySQL 8.0 does also accept TRUE/FALSE, directly confirmed, but this
    // exporter targets the one literal form every dialect in this file
    // already uses consistently).
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_mysql_sql_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "customers.dbf";
    const fs::path cdx_path = temp_dir / "customers.cdx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 145U, .length = 4U, .decimal_count = 0U}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"DATABASE", "Sales", "", ""}, {"TABLE", "customers", "Sales", ""}});
    expect(dbc_create.ok, "MySQL export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "CUST_ID", .type = 'N', .offset = 1U, .length = 6U, .decimal_count = 0U},
        {.name = "COMPANY", .type = 'C', .offset = 7U, .length = 40U, .decimal_count = 0U},
        {.name = "ACTIVE", .type = 'L', .offset = 47U, .length = 1U, .decimal_count = 0U},
        {.name = "BALANCE", .type = 'Y', .offset = 48U, .length = 8U, .decimal_count = 4U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"1001", "Acme Corp", "T", "123.45"}});
    expect(table_create.ok, "MySQL export test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_cdx_bytes_for_postgresql_index_test();
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_mysql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_mysql_sql should resolve the customers fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("EXPORT DATABASE ... TYPE MYSQL") != std::string::npos,
           "export_database_as_mysql_sql should label its own header comment as a MySQL export");
    expect(result.sql.find("CREATE TABLE `customers`") != std::string::npos,
           "export_database_as_mysql_sql should quote identifiers with backticks, matching real MySQL");
    expect(result.sql.find("`CUST_ID` DECIMAL(6, 0)") != std::string::npos,
           "export_database_as_mysql_sql should map a numeric field to DECIMAL, a real MySQL type");
    expect(result.sql.find("`COMPANY` VARCHAR(40)") != std::string::npos,
           "export_database_as_mysql_sql should map a character field to VARCHAR(length)");
    expect(result.sql.find("`ACTIVE` TINYINT(1)") != std::string::npos,
           "export_database_as_mysql_sql should map a logical field to TINYINT(1), MySQL's own real underlying type for BOOLEAN");
    expect(result.sql.find("`BALANCE` DECIMAL(19, 4)") != std::string::npos,
           "export_database_as_mysql_sql should map VFP currency to DECIMAL(19, 4), an exact match for its own 4-decimal-digit scale");
    expect(result.sql.find("INSERT INTO `customers`") != std::string::npos,
           "export_database_as_mysql_sql should emit an INSERT for the table's row");
    expect(result.sql.find("'Acme Corp'") != std::string::npos,
           "export_database_as_mysql_sql should quote a character value as a standard SQL string literal");
    expect(result.sql.find("(1001, 'Acme Corp', 1, 123.45)") != std::string::npos ||
               result.sql.find(", 1, 123.4500)") != std::string::npos,
           "export_database_as_mysql_sql should emit 1 (not the TRUE keyword) for a true logical value");
    expect(result.sql.find("TRUE") == std::string::npos && result.sql.find("FALSE") == std::string::npos,
           "export_database_as_mysql_sql must never emit the TRUE/FALSE keywords, matching this file's own cross-version-safe convention");

    expect(result.sql.find("CREATE INDEX `customers_CUST_ID_idx` ON `customers` (`CUST_ID`);") != std::string::npos,
           "export_database_as_mysql_sql should emit a CREATE INDEX for a tag whose key expression is a plain column reference");
    expect(result.sql.find("-- skipped index") != std::string::npos &&
               result.sql.find("COMPANY_N") != std::string::npos,
           "export_database_as_mysql_sql should report a composite-expression tag as a skipped index, not silently drop or mistranslate it");
    expect(result.sql.find("UPPER(company_name)") == std::string::npos,
           "export_database_as_mysql_sql must never emit a raw VFP key expression as if it were valid MySQL syntax");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_mysql_sql_omits_indexes_without_cdx() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_mysql_sql_no_cdx_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "MySQL no-CDX export test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "SKU", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"W-1"}});
    expect(table_create.ok, "MySQL no-CDX export test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_mysql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_mysql_sql should succeed without a companion CDX: " + result.error);
    expect(result.sql.find("CREATE TABLE `widgets`") != std::string::npos,
           "export_database_as_mysql_sql should still emit the table without any index information");
    expect(result.sql.find("CREATE INDEX") == std::string::npos,
           "export_database_as_mysql_sql should not emit any CREATE INDEX when no companion CDX exists");

    fs::remove_all(temp_dir, ignored);
}

// Like SQL Server (#5561) and unlike PostgreSQL/SQLite (#5559/#5558), a
// real MySQL 8.0 engine directly confirmed during this issue's own
// development that index names are scoped *per table*, not schema-wide:
// two different tables can each carry an index of the identical name
// with no error. This is a positive test locking in that real, verified
// divergence -- table "A_B"'s tag "CDEF" and table "A"'s tag "B_CDEF"
// would collide under #5559's cross-table disambiguation on the other
// two dialects, but on MySQL both must emit the exact same
// "A_B_CDEF_idx" name unchanged, since each is scoped to its own table.
void test_export_database_as_mysql_sql_allows_identical_index_name_across_tables() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_mysql_sql_same_name_across_tables_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "A_B", ""}, {"TABLE", "A", ""}});
    expect(dbc_create.ok, "MySQL same-name-across-tables test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table1_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A_B.dbf").string(), table_fields, {{"x"}});
    expect(table1_create.ok, "MySQL same-name-across-tables test: A_B.dbf fixture should be created");
    const auto table2_create = copperfin::vfp::create_dbf_table_file(
        (temp_dir / "A.dbf").string(), table_fields, {{"y"}});
    expect(table2_create.ok, "MySQL same-name-across-tables test: A.dbf fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("CDEF", "COL");
        std::ofstream output(temp_dir / "A_B.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }
    {
        const auto cdx_bytes = make_synthetic_single_tag_cdx_bytes("B_CDEF", "COL");
        std::ofstream output(temp_dir / "A.cdx", std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_mysql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_mysql_sql should resolve the same-name-across-tables fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("CREATE INDEX `A_B_CDEF_idx` ON `A_B` (`COL`);") != std::string::npos,
           "export_database_as_mysql_sql should emit the first table's own naturally-derived index name unchanged");
    expect(result.sql.find("CREATE INDEX `A_B_CDEF_idx` ON `A` (`COL`);") != std::string::npos,
           "export_database_as_mysql_sql should also emit the identical index name unchanged for the second table -- real MySQL scopes index names per table, not schema-wide");
    expect(result.sql.find("CREATE INDEX `A_B_CDEF_idx_2`") == std::string::npos,
           "export_database_as_mysql_sql must not disambiguate a cross-table index name that real MySQL does not actually collide on");

    fs::remove_all(temp_dir, ignored);
}

// Unlike PostgreSQL's silent truncation, a real MySQL 8.0 engine directly
// confirmed during this issue's own development that it *rejects* (error
// 1059) any identifier over 64 characters outright -- so this exporter
// must never construct one. A 61-character table name with two tags
// ("TAG1"/"TAG2") on the same column produces raw candidates that are
// identical for their first 64 characters (index 64 is where "TAG1"/
// "TAG2" first differ), exercising the one collision that *can* still
// occur under MySQL's real per-table scoping: truncation erasing the
// tag-derived suffix entirely when the table name alone is already at or
// beyond the limit.
void test_export_database_as_mysql_sql_disambiguates_indexes_within_identifier_length_limit() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_mysql_sql_identifier_length_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::string long_table_name(61U, 'A');
    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / (long_table_name + ".dbf");
    const fs::path cdx_path = temp_dir / (long_table_name + ".cdx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 150U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 167U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", long_table_name, ""}});
    expect(dbc_create.ok, "MySQL identifier-length test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x"}});
    expect(table_create.ok, "MySQL identifier-length test: DBF fixture should be created");

    {
        const auto cdx_bytes = make_synthetic_two_tag_cdx_bytes("TAG1", "TAG2", "COL");
        std::ofstream output(cdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(cdx_bytes.data()), static_cast<std::streamsize>(cdx_bytes.size()));
    }

    const auto result = copperfin::vfp::export_database_as_mysql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_mysql_sql should resolve the identifier-length fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    // First candidate truncates to exactly 64 characters: the 61 'A's
    // plus "_TA" (the two characters after the underscore "TAG1"/"TAG2"
    // still share before diverging at what would be index 64).
    const std::string first_truncated = long_table_name + "_TA";
    expect(first_truncated.size() == 64U,
           "test fixture sanity: the first truncated candidate should be exactly 64 characters");
    expect(result.sql.find("CREATE INDEX `" + first_truncated + "`") != std::string::npos,
           "export_database_as_mysql_sql should truncate the first candidate to MySQL's 64-character identifier limit");

    const std::string second_suffixed = long_table_name + "__2";
    expect(second_suffixed.size() == 64U,
           "test fixture sanity: the disambiguated second candidate should still be exactly 64 characters");
    expect(result.sql.find("CREATE INDEX `" + second_suffixed + "`") != std::string::npos,
           "export_database_as_mysql_sql should reserve room for the disambiguating suffix within the 64-character limit");
    expect(result.sql.find("CREATE INDEX `" + first_truncated + "`", result.sql.find("CREATE INDEX `" + first_truncated + "`") + 1U)
               == std::string::npos,
           "export_database_as_mysql_sql must never emit the exact same truncated index name twice on the same table");

    fs::remove_all(temp_dir, ignored);
}

// A blank VFP date/datetime field decodes to an empty display_value (not
// is_null). Directly confirmed against a real local MySQL 8.0 engine
// (whose default sql_mode includes STRICT_TRANS_TABLES) that
// `INSERT INTO ... VALUES ('')` into a DATE or DATETIME column fails
// outright with error 1292 ("Incorrect date/datetime value: ''") -- a
// third, distinct failure mode from SQL Server's own silent
// 1900-01-01 data corruption and Oracle's own invalid-DATE-''-syntax
// rejection, but the same NULL-instead-of-empty-string fix applies here
// too. A non-blank date/datetime must still emit as a normal quoted ISO
// string, unaffected by this fix.
void test_export_database_as_mysql_sql_preserves_blank_dates_as_null() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_mysql_sql_blank_date_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "events.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "events", ""}});
    expect(dbc_create.ok, "MySQL blank-date test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "OCCURRED", .type = 'D', .offset = 1U, .length = 8U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"20260115"}, {""}});
    expect(table_create.ok, "MySQL blank-date test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_mysql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_mysql_sql should resolve the blank-date fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("VALUES ('2026-01-15')") != std::string::npos,
           "export_database_as_mysql_sql should emit a non-blank date as a normal quoted ISO string literal");
    expect(result.sql.find("VALUES (NULL)") != std::string::npos,
           "export_database_as_mysql_sql should emit NULL for a blank date rather than an empty string literal");
    expect(result.sql.find("VALUES ('')") == std::string::npos,
           "export_database_as_mysql_sql must never emit an empty string literal for a DATE column -- real MySQL's own strict sql_mode rejects it outright (error 1292)");

    fs::remove_all(temp_dir, ignored);
}

// A crafted or corrupt DBF header does not have to keep length/decimal_count
// within MySQL's own valid ranges the way a table genuinely written by
// this codebase's own writer always does. Directly confirmed against a
// real local MySQL 8.0 engine: DECIMAL(65, 0) succeeds while DECIMAL(66, 0)
// fails ("Too-big precision 66... Maximum is 65"), DECIMAL(65, 30)
// succeeds while DECIMAL(10, 31) fails ("Too big scale 31... Maximum is
// 30"), and DECIMAL(10, 20) -- scale exceeding precision -- fails
// outright ("M must be >= D"), a genuine difference from Oracle's own
// independent scale range and matching SQL Server's own clamp-scale-to-
// precision requirement instead. Exercises both clamp boundaries in one
// table: a (255, 255) descriptor proves the 65/30 ceilings, and a
// (10, 255) descriptor -- whose precision is well under 65 -- proves
// scale is additionally clamped to that (already-clamped) precision, not
// just to the flat 30-digit ceiling.
void test_export_database_as_mysql_sql_clamps_decimal_precision_and_scale() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_mysql_sql_decimal_clamp_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "amounts.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "amounts", ""}});
    expect(dbc_create.ok, "MySQL DECIMAL-clamp test: DBC fixture should be created");

    // A genuinely valid DBF numeric field can never carry length/decimal_count
    // this large (VFP's own N field caps at 20 digits) -- construct the
    // descriptors directly (bypassing create_dbf_table_file's own writer,
    // which would refuse this) to model a crafted/corrupt header.
    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "AMOUNT", .type = 'N', .offset = 1U, .length = 255U, .decimal_count = 255U},
        {.name = "SMALLAMT", .type = 'N', .offset = 2U, .length = 10U, .decimal_count = 255U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"0", "0"}});
    expect(table_create.ok, "MySQL DECIMAL-clamp test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_mysql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_mysql_sql should resolve the DECIMAL-clamp fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("`AMOUNT` DECIMAL(65, 30)") != std::string::npos,
           "export_database_as_mysql_sql should clamp precision to MySQL's 65-digit maximum and scale to its own 30-digit maximum");
    expect(result.sql.find("`SMALLAMT` DECIMAL(10, 10)") != std::string::npos,
           "export_database_as_mysql_sql should additionally clamp scale to a smaller precision when that precision is itself under 30, since real MySQL rejects a DECIMAL whose scale exceeds its own precision");
    expect(result.sql.find("DECIMAL(255") == std::string::npos,
           "export_database_as_mysql_sql must never emit a DECIMAL precision real MySQL rejects");

    fs::remove_all(temp_dir, ignored);
}

// #5554: unlike every other dialect this file emits -- all of which
// reuse the shared sql_quote_string_literal()'s plain doubled-single-
// quote escaping -- MySQL's own default sql_mode (no
// NO_BACKSLASH_ESCAPES) treats a backslash as a live escape character
// inside a string literal. Directly confirmed against a real local MySQL
// 8.0 engine: inserting the literal `'C:\temp''s file'` (plain ANSI-style
// doubled-quote escaping) stores only 13 characters, not 14 -- the `\t`
// was silently interpreted as a TAB character rather than a literal
// backslash followed by `t`. Doubling the backslash too (`'C:\\temp''s
// file'`) stores the correct, literal 14-character string. A Windows
// path inside a VFP character or memo field is a genuinely common case
// for this codebase, not a contrived one.
void test_export_database_as_mysql_sql_escapes_backslash_in_string_literals() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_mysql_sql_backslash_escape_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "paths.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "paths", ""}});
    expect(dbc_create.ok, "MySQL backslash-escape test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "WINPATH", .type = 'C', .offset = 1U, .length = 30U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields,
        {{"C:\\temp's file"}});
    expect(table_create.ok, "MySQL backslash-escape test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_mysql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_mysql_sql should resolve the backslash-escape fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("'C:\\\\temp''s file'") != std::string::npos,
           "export_database_as_mysql_sql should double both the embedded backslash and the embedded single quote, since real MySQL's own default sql_mode treats a lone backslash as a live escape character inside a string literal");
    expect(result.sql.find("'C:\\temp''s file'") == std::string::npos,
           "export_database_as_mysql_sql must never emit a single un-doubled backslash inside a string literal -- real MySQL would silently reinterpret it as an escape sequence rather than store it literally");

    fs::remove_all(temp_dir, ignored);
}

// A genuinely valid VFP field name can never contain a literal backtick --
// construct the descriptor directly to model a crafted/corrupt source,
// matching this file's own established practice (see
// test_export_database_as_oracle_sql_strips_embedded_quote_from_identifiers()
// above) for exercising fail-closed/lossy-but-safe handling of input a
// real writer never produces. Unlike Oracle's own lossy quote-stripping,
// MySQL provides a real escape mechanism for an embedded backtick
// (doubling it, like every dialect this file emits except Oracle) --
// directly confirmed against a real local MySQL 8.0 engine that
// `` `weird``name` `` round-trips losslessly through CREATE TABLE.
void test_export_database_as_mysql_sql_escapes_embedded_backtick_in_identifiers() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_mysql_sql_embedded_backtick_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "widgets.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "widgets", ""}});
    expect(dbc_create.ok, "MySQL embedded-backtick test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "weird`name", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x"}});
    expect(table_create.ok, "MySQL embedded-backtick test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_mysql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(result.ok, "export_database_as_mysql_sql should resolve the embedded-backtick fixture: " + result.error);
    if (!result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    expect(result.sql.find("`weird``name`") != std::string::npos,
           "export_database_as_mysql_sql should escape an embedded backtick by doubling it, since real MySQL provides a real (lossless) escape mechanism for one, unlike Oracle's own embedded double quote");

    fs::remove_all(temp_dir, ignored);
}

// #5582 PR review (chatgpt-codex-connector, P2): a DBC catalog's own
// OBJECTNAME column permits a table name longer than MySQL's real
// 64-character identifier limit -- directly confirmed against a real
// local MySQL 8.0 engine that a 65-character identifier fails outright
// with error 1059. Without a check, export_database_as_mysql_sql() would
// return ok=true for a script whose first CREATE TABLE a real MySQL
// engine rejects. A 65-character table name (one character past the
// limit) must fail the whole export closed rather than emit an unusable
// script.
void test_export_database_as_mysql_sql_fails_closed_on_overlong_table_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_mysql_sql_overlong_table_name_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::string long_table_name(65U, 'A');
    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / (long_table_name + ".dbf");
    // OBJECTNAME must be wide enough to hold the 65-character long_table_name
    // itself -- a real VFP table name can never be this long, but the DBC
    // catalog's own OBJECTNAME field is a plain sizable Character column a
    // crafted or foreign-tool-written catalog is not bound to keep short.
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 100U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 117U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", long_table_name, ""}});
    expect(dbc_create.ok, "MySQL overlong-table-name test: DBC fixture should be created");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "COL", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U},
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(table_path), table_fields, {{"x"}});
    expect(table_create.ok, "MySQL overlong-table-name test: DBF fixture should be created");

    const auto result = copperfin::vfp::export_database_as_mysql_sql(
        copperfin::platform::path_to_utf8_string(dbc_path));
    expect(!result.ok,
           "export_database_as_mysql_sql should fail closed when a table name exceeds MySQL's 64-character identifier limit, rather than emit a script real MySQL rejects");
    expect(result.sql.empty(),
           "export_database_as_mysql_sql should not return a partial SQL script alongside a failure");
    expect(result.error.find(long_table_name) != std::string::npos,
           "export_database_as_mysql_sql's identifier-too-long error should name the offending identifier: " + result.error);

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

// #5697: a member DBF whose field-descriptor block parses "successfully"
// with zero fields (an immediately-encountered terminator, a real shape
// parse_dbf_field_descriptor_block() accepts) previously let every
// exporter either emit unusable output (invalid `CREATE TABLE ( );` DDL
// on every SQL dialect) or an ambiguous marker (JSON's `"fields": []`,
// indistinguishable from its own deliberate unreadable-table marker) and
// still report the migration successful. This fixture is a raw-byte
// zero-field DBF (an aligned terminator immediately after the 32-byte
// header, matching parse_dbf_field_descriptor_block()'s own accepted
// shape) rather than one built through create_dbf_table_file(), which
// already rejects an empty field list on the write side and so cannot
// produce this fixture itself. Verified to reliably fail against the
// pre-fix code (each exporter previously returned ok=true here) and
// reliably pass against the fix, for all eight EXPORT DATABASE TYPE
// variants that share this one class of gap.
void test_export_database_family_fails_closed_on_zero_field_table() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_zero_field_export_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path table_path = temp_dir / "empty.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 64U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 81U, .length = 64U, .decimal_count = 0U},
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(dbc_path), dbc_fields,
        {{"TABLE", "empty", ""}});
    expect(dbc_create.ok, "zero-field export test: DBC fixture should be created");

    // Raw VFP-style DBF bytes: 32-byte header, header_length = 33 (an
    // aligned field-descriptor terminator immediately at offset 32, zero
    // descriptors between), record_length = 1 (delete flag only), zero
    // records.
    std::vector<std::uint8_t> table_bytes(34U, 0U);
    table_bytes[0] = 0x30U;  // VFP version byte
    write_le_u32(table_bytes, 4U, 0U);   // record count = 0
    write_le_u16(table_bytes, 8U, 33U);  // header_length
    write_le_u16(table_bytes, 10U, 1U);  // record_length
    table_bytes[32U] = 0x0DU;  // field-descriptor terminator -> zero fields
    table_bytes[33U] = 0x1AU;  // EOF marker
    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()),
                      static_cast<std::streamsize>(table_bytes.size()));
    }

    const std::string dbc_utf8 = copperfin::platform::path_to_utf8_string(dbc_path);

    const auto json_result = copperfin::vfp::export_database_as_json(dbc_utf8);
    expect(!json_result.ok,
           "export_database_as_json must fail closed on a zero-field member table, not report a usable schema");

    const auto sql_result = copperfin::vfp::export_database_as_sql(dbc_utf8);
    expect(!sql_result.ok,
           "export_database_as_sql must fail closed on a zero-field member table rather than emit invalid CREATE TABLE ( ) DDL");

    const auto postgresql_result = copperfin::vfp::export_database_as_postgresql_sql(dbc_utf8);
    expect(!postgresql_result.ok,
           "export_database_as_postgresql_sql must fail closed on a zero-field member table");

    const auto sqlite_result = copperfin::vfp::export_database_as_sqlite_sql(dbc_utf8);
    expect(!sqlite_result.ok,
           "export_database_as_sqlite_sql must fail closed on a zero-field member table");

    const auto sqlserver_result = copperfin::vfp::export_database_as_sqlserver_sql(dbc_utf8);
    expect(!sqlserver_result.ok,
           "export_database_as_sqlserver_sql must fail closed on a zero-field member table");

    const auto oracle_result = copperfin::vfp::export_database_as_oracle_sql(dbc_utf8);
    expect(!oracle_result.ok,
           "export_database_as_oracle_sql must fail closed on a zero-field member table");

    const auto mysql_result = copperfin::vfp::export_database_as_mysql_sql(dbc_utf8);
    expect(!mysql_result.ok,
           "export_database_as_mysql_sql must fail closed on a zero-field member table");

    const auto access_result = copperfin::vfp::export_database_as_access_sql(dbc_utf8);
    expect(!access_result.ok,
           "export_database_as_access_sql must fail closed on a zero-field member table");

    fs::remove_all(temp_dir, ignored);
}

// #5697: validate_dbf_field_descriptors() (src/vfp/asset_inspector.cpp)
// previously returned silently, with no diagnostic at all, when an aligned
// field-descriptor terminator was found immediately at the descriptor
// start -- a real, accepted shape (matching parse_dbf_field_descriptor_
// block()'s own permissive parse), but one that produces a schema with no
// usable columns. Verified to reliably fail against the pre-fix code (no
// validation issue at all) and reliably pass against the fix.
void test_inspect_asset_reports_zero_field_dbf_as_error() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_zero_field_dbf_inspect_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "empty.dbf";
    std::vector<std::uint8_t> table_bytes(34U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 0U);
    write_le_u16(table_bytes, 8U, 33U);
    write_le_u16(table_bytes, 10U, 1U);
    table_bytes[32U] = 0x0DU;
    table_bytes[33U] = 0x1AU;
    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()),
                      static_cast<std::streamsize>(table_bytes.size()));
    }

    const auto result = copperfin::vfp::inspect_asset(copperfin::platform::path_to_utf8_string(table_path));
    expect(has_validation_issue(result, "dbf.field_count_zero"),
           "inspect_asset should report a zero-field DBF schema as a validation error");

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
    test_scan_access_saved_queries_reconstructs_select_from_where();
    test_scan_access_saved_queries_skips_query_with_undecodable_clause_text();
    test_scan_access_saved_queries_appends_multiple_order_by_expressions();
    test_scan_access_saved_queries_skips_query_with_no_table_reference();
    test_scan_access_saved_queries_reports_ok_with_no_queries();
    test_create_vfp_cdx_single_tag_index_file_writes_readable_tag_and_sets_production_index_flag();
    test_create_vfp_cdx_single_tag_index_file_rejects_empty_tag_name();
    test_create_vfp_cdx_single_tag_index_file_rejects_key_exceeding_declared_length();
    test_create_vfp_cdx_single_tag_index_file_rejects_key_longer_than_compression_encoding_limit();
    test_create_vfp_cdx_single_tag_index_file_rejects_record_number_out_of_range();
    test_create_vfp_cdx_single_tag_index_file_rejects_invalid_key_length();
    test_create_vfp_cdx_single_tag_index_file_rejects_oversized_key_expression();
    test_create_vfp_cdx_single_tag_index_file_rejects_tag_name_that_would_overwrite_header();
    test_create_vfp_cdx_single_tag_index_file_does_not_set_production_index_flag_when_cdx_write_fails();
    test_create_vfp_cdx_single_tag_index_file_rejects_leaf_page_overflow();
    test_parse_access_saveastext_design_parses_form_control_hierarchy();
    test_parse_access_saveastext_design_accepts_report_root_type();
    test_parse_access_saveastext_design_unescapes_multiline_string_continuation();
    test_parse_access_saveastext_design_rejects_malformed_header();
    test_parse_access_saveastext_design_rejects_unbalanced_block();
    test_parse_access_saveastext_design_rejects_unsupported_root_type();
    test_parse_access_saveastext_design_rejects_malformed_property_line();
    test_parse_access_saveastext_design_rejects_unterminated_string();
    test_parse_access_saveastext_design_rejects_unexpected_trailing_content();
    test_parse_access_saveastext_design_rejects_trailing_content_after_blank_line();
    test_parse_access_saveastext_design_finds_code_behind_after_blank_line();
    test_parse_access_saveastext_design_preserves_crlf_in_code_behind();
    test_parse_access_saveastext_design_preserves_blob_line_whitespace();
    test_access_saveastext_design_find_property_returns_null_when_absent();
    test_parse_access_saveastext_design_from_file_reports_open_failure();
    test_parse_access_long_value_field_descriptor_decodes_header();
    test_parse_access_long_value_field_descriptor_rejects_short_input();
    test_read_access_long_value_column_returns_inline_value();
    test_read_access_long_value_column_rejects_inline_length_mismatch();
    test_read_access_long_value_column_rejects_descriptor_too_short();
    test_read_access_long_value_column_rejects_unsupported_bitmask();
    test_read_access_long_value_column_reads_single_lval_page();
    test_read_access_long_value_column_reads_chained_lval_pages();
    test_read_access_long_value_column_rejects_page_without_lval_marker();
    test_read_access_long_value_column_rejects_row_index_out_of_range();
    test_read_access_long_value_column_rejects_deleted_row_slot();
    test_read_access_long_value_column_rejects_cyclic_chain();
    test_read_access_long_value_column_rejects_length_overrun();
    test_read_access_long_value_column_rejects_later_generation();
    test_access_container_errors_resolve_through_localization_catalog();
    test_vfp_locale_catalog_parity();
    test_inspect_database_container_collects_casefolded_same_base_companions();
    test_inspect_database_container_extracts_first_pass_catalog_metadata();
    test_extract_dbc_stored_procedures_source_reads_code_memo();
    test_extract_dbc_stored_procedures_source_absent_is_not_an_error();
    test_extract_dbc_stored_procedures_source_fails_closed_without_memo_sidecar();
    test_inspect_asset_resolves_explicit_unicode_memo_sidecar();
    test_export_database_as_json_resolves_unicode_catalog_table_path();
    test_export_database_as_json_rejects_dotdot_table_name_traversal();
    test_export_database_as_json_rejects_absolute_table_name();
#if !defined(_WIN32)
    test_export_database_as_json_rejects_symlink_table_escaping_directory();
    test_export_database_as_json_rejects_symlink_memo_sidecar_escaping_directory();
#endif
    test_export_database_as_sql_maps_currency_datetime_and_blank_numeric();
    test_export_database_as_sql_preserves_exponent_form_double_values();
    test_export_database_as_json_fails_closed_on_numeric_structural_injection();
    test_export_database_as_json_fails_closed_on_unsafe_numeric_forms();
    test_export_database_as_json_still_accepts_valid_numeric_forms();
    test_export_database_as_json_escapes_crafted_field_type_byte();
    test_export_database_as_sql_family_preserves_blank_dates_as_null();
    test_export_database_as_postgresql_sql_maps_types_and_creates_indexes();
    test_export_database_as_postgresql_sql_omits_indexes_without_cdx();
    test_export_database_as_postgresql_sql_disambiguates_indexes_on_same_column();
    test_export_database_as_postgresql_sql_disambiguates_indexes_within_identifier_length_limit();
    test_export_database_as_postgresql_sql_disambiguates_indexes_across_tables();
    test_export_database_as_postgresql_sql_disambiguates_index_colliding_with_table_name();
    test_export_database_as_sqlite_sql_maps_types_and_creates_indexes();
    test_export_database_as_sqlite_sql_omits_indexes_without_cdx();
    test_export_database_as_sqlserver_sql_maps_types_and_creates_indexes();
    test_export_database_as_sqlserver_sql_omits_indexes_without_cdx();
    test_export_database_as_sqlserver_sql_allows_identical_index_name_across_tables();
    test_export_database_as_sqlserver_sql_disambiguates_indexes_within_identifier_length_limit();
    test_export_database_as_sqlserver_sql_preserves_blank_dates_as_null();
    test_export_database_as_sqlserver_sql_clamps_decimal_precision_and_scale();
    test_export_database_as_sqlserver_sql_truncates_by_unicode_character_not_byte_count();
    test_export_database_as_postgresql_sql_truncates_on_utf8_character_boundary();
    test_export_database_as_oracle_sql_maps_types_and_creates_indexes();
    test_export_database_as_oracle_sql_omits_indexes_without_cdx();
    test_export_database_as_oracle_sql_disambiguates_indexes_across_tables();
    test_export_database_as_oracle_sql_allows_index_named_after_a_table();
    test_export_database_as_oracle_sql_disambiguates_indexes_within_identifier_length_limit();
    test_export_database_as_oracle_sql_preserves_blank_dates_as_null();
    test_export_database_as_oracle_sql_clamps_number_precision_and_scale();
    test_export_database_as_oracle_sql_strips_embedded_quote_from_identifiers();
    test_export_database_as_oracle_sql_declares_character_columns_with_char_semantics();
    test_export_database_as_oracle_sql_writes_memo_content_as_clob_literals();
    test_export_database_as_oracle_sql_fails_closed_on_colliding_identifiers();
    test_export_database_as_oracle_sql_fails_closed_on_non_null_empty_character_value();
    test_export_database_as_oracle_sql_allows_blank_value_in_table_with_nullable_fields();
    test_export_database_as_mysql_sql_maps_types_and_creates_indexes();
    test_export_database_as_mysql_sql_omits_indexes_without_cdx();
    test_export_database_as_mysql_sql_allows_identical_index_name_across_tables();
    test_export_database_as_mysql_sql_disambiguates_indexes_within_identifier_length_limit();
    test_export_database_as_mysql_sql_preserves_blank_dates_as_null();
    test_export_database_as_mysql_sql_clamps_decimal_precision_and_scale();
    test_export_database_as_mysql_sql_escapes_backslash_in_string_literals();
    test_export_database_as_mysql_sql_escapes_embedded_backtick_in_identifiers();
    test_export_database_as_mysql_sql_fails_closed_on_overlong_table_name();
    test_export_database_as_sqlite_sql_disambiguates_indexes_across_tables();
    test_export_database_as_access_sql_maps_currency_datetime_and_dates();
    test_export_database_as_access_sql_escapes_bracket_in_identifier();
    test_export_database_as_access_sql_clamps_decimal_precision();
    test_export_database_as_access_sql_rejects_unsafe_numeric_token();
    test_export_database_as_access_sql_accepts_leading_plus_sign_numeric();
    test_export_database_as_access_sql_rejects_unsafe_date_literal();
    test_database_json_import_plan_admits_exporter_unreadable_table_marker();
    test_export_database_family_fails_closed_on_zero_field_table();
    test_inspect_asset_reports_zero_field_dbf_as_error();
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
