#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/index_probe.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

bool has_validation_issue(
    const copperfin::vfp::AssetInspectionResult& result,
    const std::string& code,
    const std::string& path_suffix = {}) {
    return std::any_of(
        result.validation_issues.begin(),
        result.validation_issues.end(),
        [&](const copperfin::vfp::AssetValidationIssue& issue) {
            if (issue.code != code) {
                return false;
            }
            if (path_suffix.empty()) {
                return true;
            }
            return issue.path.size() >= path_suffix.size() &&
                   issue.path.ends_with(path_suffix);
        });
}

std::vector<std::uint8_t> make_vfp_header() {
    std::vector<std::uint8_t> bytes(32U, 0U);
    bytes[0] = 0x30U;
    bytes[1] = 126U;
    bytes[2] = 4U;
    bytes[3] = 7U;
    bytes[4] = 10U;
    bytes[8] = 0xA1U;
    bytes[9] = 0x00U;
    bytes[10] = 0x40U;
    bytes[11] = 0x00U;
    bytes[28] = 0x05U;
    bytes[29] = 0x03U;
    return bytes;
}

void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value);
void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value);
void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value);
void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value);
void write_ascii(std::vector<std::uint8_t>& bytes, std::size_t offset, const std::string& value);
void write_field_descriptor(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::string& name,
    char type,
    std::uint32_t field_offset,
    std::uint8_t length);

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
        write_ascii(bytes, (3U * 512U) - 20U, "CUSTOMER_I");
        write_ascii(bytes, (3U * 512U) - 10U, "COMPANY_NA");
        write_ascii(bytes, 4U * 512U, "UPPER(company_name)");
        write_ascii(bytes, 11U * 512U, "customer_id");
    } else {
        write_ascii(bytes, (3U * 512U) - 10U, "NAME");
        write_ascii(bytes, 4U * 512U, "UPPER(NAME)");
    }

    if (include_for_expression) {
        write_ascii(bytes, 5U * 512U, "DELETED() = .F.");
    }

    return bytes;
}

void test_parse_dbf_header() {
    const auto bytes = make_vfp_header();
    const auto result = copperfin::vfp::parse_dbf_header(bytes);

    expect(result.ok, "parse_dbf_header should succeed for a valid synthetic header");
    expect(result.header.version == 0x30U, "version should be parsed");
    expect(result.header.record_count == 10U, "record_count should be parsed");
    expect(result.header.header_length == 161U, "header_length should be parsed");
    expect(result.header.record_length == 64U, "record_length should be parsed");
    expect(result.header.has_structural_cdx(), "structural CDX flag should be detected");
    expect(result.header.has_database_container(), "database container flag should be detected");
    expect(result.header.version_description() == "Visual FoxPro", "version description should match Visual FoxPro");
    expect(result.header.last_update_iso8601() == "2026-04-07", "last update date should be formatted as ISO 8601");
}

void test_parse_dbf_header_rejects_short_input() {
    const auto result = copperfin::vfp::parse_dbf_header({0x30U, 0x00U});
    expect(!result.ok, "parse_dbf_header should reject short input");
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

void test_parse_index_probe_for_cdx() {
    const auto bytes = make_synthetic_cdx_family_bytes(true, true);

    const auto result = copperfin::vfp::parse_index_probe(bytes, 16U * 512U, copperfin::vfp::IndexKind::cdx);
    expect(result.ok, "parse_index_probe should succeed for a plausible synthetic CDX header");
    expect(result.probe.root_node_offset_hint == 1024U, "CDX root node offset should be parsed");
    expect(result.probe.key_length_hint == 10U, "CDX key length hint should be parsed");
    expect(result.probe.group_length_hint == 480U, "CDX key pool length hint should be parsed");
    expect(result.probe.multi_tag, "CDX should be treated as multi-tag");
    expect(result.probe.tags.size() == 2U, "CDX probe should enumerate tags from directory leaf pages");
    expect(result.probe.for_expression_hint == "DELETED() = .F.", "CDX probe should surface the first tag FOR expression");
    if (result.probe.tags.size() >= 2U) {
        expect(result.probe.tags[0].name_hint == "CUSTOMER_I", "directory leaf parsing should preserve the first stored tag name");
        expect(
            result.probe.tags[0].key_expression_hint == "customer_id",
            "directory tag names should still bind to the matching plain-field expression");
        expect(result.probe.tags[0].for_expression_hint.empty(), "tags should not borrow FOR expressions from a different key-expression span");
        expect(!result.probe.tags[0].inferred_name, "directory-derived tag names should not be marked as inferred");
        expect(result.probe.tags[1].name_hint == "COMPANY_NA", "directory leaf parsing should preserve the second stored tag name");
        expect(
            result.probe.tags[1].key_expression_hint == "UPPER(company_name)",
            "directory tag names should still bind to the matching functional expression");
        expect(
            result.probe.tags[1].for_expression_hint == "DELETED() = .F.",
            "page-local FOR expressions should attach to the matching CDX tag");
        expect(!result.probe.tags[1].inferred_name, "directory-derived tag names should not be marked as inferred");
    }
}

void test_parse_index_probe_for_dcx() {
    const auto bytes = make_synthetic_cdx_family_bytes(false, true);

    const auto result = copperfin::vfp::parse_index_probe(bytes, 16U * 512U, copperfin::vfp::IndexKind::dcx);
    expect(result.ok, "parse_index_probe should succeed for a plausible synthetic DCX header");
    expect(result.probe.kind == copperfin::vfp::IndexKind::dcx, "DCX probe kind should be preserved");
    expect(result.probe.multi_tag, "DCX should be treated as multi-tag");
    expect(!result.probe.production_candidate, "DCX should not be flagged as a table production index");
    expect(result.probe.tags.size() == 1U, "DCX probe should reuse the shared CDX-family tag parser");
    if (!result.probe.tags.empty()) {
        expect(result.probe.tags.front().name_hint == "NAME", "DCX probe should preserve the stored tag name");
        expect(result.probe.tags.front().key_expression_hint == "UPPER(NAME)", "DCX probe should expose the key expression hint");
        expect(result.probe.tags.front().for_expression_hint == "DELETED() = .F.", "DCX probe should expose the FOR expression hint");
    }
}

void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_ascii(std::vector<std::uint8_t>& bytes, std::size_t offset, const std::string& value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>(value[index]);
    }
}

void write_field_descriptor(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::string& name,
    char type,
    std::uint32_t field_offset,
    std::uint8_t length) {
    for (std::size_t index = 0; index < 11U && index < name.size(); ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>(name[index]);
    }
    bytes[offset + 11U] = static_cast<std::uint8_t>(type);
    write_le_u32(bytes, offset + 12U, field_offset);
    bytes[offset + 16U] = length;
}

void test_parse_index_probe_for_idx() {
    std::vector<std::uint8_t> bytes(512U, 0U);
    write_le_u32(bytes, 0U, 512U);
    write_le_u32(bytes, 4U, 0xFFFFFFFFU);
    write_le_u32(bytes, 8U, 1024U);
    write_le_u16(bytes, 12U, 10U);
    bytes[14] = 0x01U;
    write_ascii(bytes, 16U, "UPPER(NAME)");
    write_ascii(bytes, 236U, "DELETED() = .F.");

    const auto result = copperfin::vfp::parse_index_probe(bytes, 1024U, copperfin::vfp::IndexKind::idx);
    expect(result.ok, "parse_index_probe should succeed for a plausible Visual FoxPro IDX header");
    expect(result.probe.kind == copperfin::vfp::IndexKind::idx, "IDX probe kind should be preserved");
    expect(result.probe.root_node_offset_hint == 512U, "IDX root node offset should be parsed");
    expect(result.probe.end_of_file_offset_hint == 1024U, "IDX end-of-file offset should be parsed");
    expect(result.probe.key_length_hint == 10U, "IDX key length should be parsed");
    expect(result.probe.key_expression_hint == "UPPER(NAME)", "IDX key expression should be extracted");
    expect(result.probe.for_expression_hint == "DELETED() = .F.", "IDX FOR expression should be extracted");
}

void test_parse_index_probe_for_ndx() {
    std::vector<std::uint8_t> bytes(512U, 0U);
    write_le_u32(bytes, 0U, 1U);
    write_le_u32(bytes, 4U, 2U);
    write_le_u16(bytes, 12U, 2U);
    write_le_u16(bytes, 14U, 42U);
    write_le_u16(bytes, 18U, 12U);
    write_le_u16(bytes, 22U, 1U);
    write_ascii(bytes, 24U, "CODE");

    const auto result = copperfin::vfp::parse_index_probe(bytes, 1024U, copperfin::vfp::IndexKind::ndx);
    expect(result.ok, "parse_index_probe should succeed for a plausible dBase NDX header");
    expect(result.probe.kind == copperfin::vfp::IndexKind::ndx, "NDX probe kind should be preserved");
    expect(result.probe.root_node_offset_hint == 512U, "NDX root block should convert to a byte offset");
    expect(result.probe.end_of_file_offset_hint == 1024U, "NDX EOF block should convert to a byte offset");
    expect(result.probe.max_keys_hint == 42U, "NDX max keys should be parsed");
    expect(result.probe.group_length_hint == 12U, "NDX group length should be parsed");
    expect(result.probe.flags == 0x01U, "NDX uniqueness flag should be projected into flags");
    expect(result.probe.key_expression_hint == "CODE", "NDX expression should be extracted");
}

void test_parse_index_probe_for_mdx() {
    std::vector<std::uint8_t> bytes(1024U, 0U);
    write_ascii(bytes, 512U, "NAME_TAG");
    write_ascii(bytes, 640U, "CITYSTATE");

    const auto result = copperfin::vfp::parse_index_probe(bytes, 1024U, copperfin::vfp::IndexKind::mdx);
    expect(result.ok, "parse_index_probe should succeed for a plausible dBase MDX file");
    expect(result.probe.kind == copperfin::vfp::IndexKind::mdx, "MDX probe kind should be preserved");
    expect(result.probe.multi_tag, "MDX should be treated as multi-tag");
    expect(result.probe.production_candidate, "MDX should be treated as a production index candidate");
    expect(result.probe.tags.size() == 2U, "MDX probe should enumerate first-pass tag hints");
    if (result.probe.tags.size() >= 2U) {
        expect(result.probe.tags[0].name_hint == "NAME_TAG", "MDX probe should expose the first tag hint");
        expect(result.probe.tags[1].name_hint == "CITYSTATE", "MDX probe should expose the second tag hint");
    }
}

void test_inspect_asset_collects_companion_indexes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_vfp_assets_tests";
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "sample.dbf";
    const fs::path cdx_path = temp_dir / "sample.cdx";
    const fs::path ndx_path = temp_dir / "sample.ndx";
    const fs::path mdx_path = temp_dir / "sample.mdx";

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
        std::vector<std::uint8_t> bytes(1024U, 0U);
        write_ascii(bytes, 512U, "NAME_TAG");
        write_ascii(bytes, 640U, "CITYSTATE");
        std::ofstream output(mdx_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto result = copperfin::vfp::inspect_asset(table_path.string());
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

    const auto write_form_with_memo_pointer = [&](const fs::path& table_path, std::uint32_t block_pointer) {
        std::vector<std::uint8_t> table_bytes(115U, 0U);
        table_bytes[0] = 0x30U;
        table_bytes[1] = 126U;
        table_bytes[2] = 4U;
        table_bytes[3] = 11U;
        write_le_u32(table_bytes, 4U, 1U);
        write_le_u16(table_bytes, 8U, 97U);
        write_le_u16(table_bytes, 10U, 18U);
        table_bytes[28U] = 0x00U;
        table_bytes[29U] = 0x03U;
        write_field_descriptor(table_bytes, 32U, "OBJNAME", 'C', 1U, 12U);
        write_field_descriptor(table_bytes, 64U, "PROPERTIES", 'M', 13U, 4U);
        table_bytes[96U] = 0x0DU;
        table_bytes[97U] = 0x20U;
        write_ascii(table_bytes, 98U, "txtTitle");
        write_le_u32(table_bytes, 110U, block_pointer);
        table_bytes.back() = 0x1AU;

        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
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

    fs::remove_all(temp_dir, ignored);
}

}  // namespace

int main() {
    test_parse_dbf_header();
    test_parse_dbf_header_rejects_short_input();
    test_asset_family_detection();
    test_parse_index_probe_for_cdx();
    test_parse_index_probe_for_dcx();
    test_parse_index_probe_for_idx();
    test_parse_index_probe_for_ndx();
    test_parse_index_probe_for_mdx();
    test_inspect_asset_collects_companion_indexes();
    test_inspect_database_container_collects_dcx_companion();
    test_parse_real_vfp_cdx_when_available();
    test_inspect_asset_reports_dbf_storage_validation_findings();
    test_inspect_asset_reports_missing_companions_and_unparseable_indexes();
    test_inspect_asset_reports_malformed_memo_sidecar_findings();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
