// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/platform/path.h"
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
    copperfin::vfp::DbfHeader foxbase_header;
    foxbase_header.version = 0x02U;
    expect(foxbase_header.format_family() == copperfin::vfp::DbfFormatFamily::foxbase,
           "FoxBase version byte should classify as the foxbase inspection family");
    copperfin::vfp::DbfHeader unknown_header;
    unknown_header.version = 0x7FU;
    expect(unknown_header.format_family() == copperfin::vfp::DbfFormatFamily::unknown,
           "unrecognized version bytes should remain explicitly unknown");
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

    const auto result = copperfin::vfp::inspect_asset(
        copperfin::platform::path_to_utf8_string(table_path));
    expect(result.ok, "inspect_asset should succeed for a synthetic DBF with companion indexes");
    expect(result.header_available, "inspect_asset should expose the DBF header");
    expect(result.indexes.size() == 3U, "inspect_asset should collect same-base CDX, NDX, and MDX companions");

    bool saw_cdx = false;
    bool saw_ndx = false;
    bool saw_mdx = false;
    for (const auto& index : result.indexes) {
        saw_cdx = saw_cdx || index.probe.kind == copperfin::vfp::IndexKind::cdx;
        saw_ndx = saw_ndx || index.probe.kind == copperfin::vfp::IndexKind::ndx;
        saw_mdx = saw_mdx || index.probe.kind == copperfin::vfp::IndexKind::mdx;
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

    std::error_code ignored;
    fs::remove(table_path, ignored);
    fs::remove(cdx_path, ignored);
    fs::remove(ndx_path, ignored);
    fs::remove(mdx_path, ignored);
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

void test_vfp_locale_catalog_parity() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> keys{
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
        "Vfp.DbfHeader.Version.FoxProMemo",
        "Vfp.DbfHeader.Version.Foxbase",
        "Vfp.DbfHeader.Version.Unknown",
        "Vfp.DbfHeader.Version.VisualFoxPro",
        "Vfp.DbfHeader.Version.VisualFoxProAutoincrement",
        "Vfp.DbfHeader.Version.VisualFoxProVarbinaryVarchar",
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
    test_index_probe_errors_resolve_through_localization_catalog();
    test_inspect_asset_collects_companion_indexes();
    test_inspect_asset_uses_admitted_index_bytes();
    test_inspect_asset_discovers_virtual_casefolded_index_bytes();
    test_inspect_database_container_collects_dcx_companion();
    test_vfp_locale_catalog_parity();
    test_inspect_database_container_collects_casefolded_same_base_companions();
    test_inspect_database_container_extracts_first_pass_catalog_metadata();
    test_inspect_asset_resolves_explicit_unicode_memo_sidecar();
    test_export_database_as_json_resolves_unicode_catalog_table_path();
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
    test_read_memo_block_raw_returns_correct_bytes();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
