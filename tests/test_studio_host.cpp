#include "copperfin/studio/document_model.h"
#include "copperfin/studio/vs_launch_contract.h"

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

std::vector<std::uint8_t> make_vfp_header() {
    std::vector<std::uint8_t> bytes(32U, 0U);
    bytes[0] = 0x30U;
    bytes[1] = 126U;
    bytes[2] = 4U;
    bytes[3] = 7U;
    bytes[8] = 0xA1U;
    bytes[9] = 0x00U;
    bytes[10] = 0x40U;
    bytes[11] = 0x00U;
    bytes[28] = 0x01U;
    bytes[29] = 0x03U;
    return bytes;
}

void test_parse_launch_arguments() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--from-vs",
        "--read-only",
        "--json",
        "--set-property",
        "--record", "3",
        "--property-name", "Left",
        "--property-value", "25",
        "--line", "25",
        "--column", "7",
        "--symbol", "cmdSave.Click"
    });

    expect(result.ok, "launch contract should parse a complete Visual Studio launch request");
    expect(result.request.path == "E:\\Forms\\customer.scx", "launch path should be captured");
    expect(result.request.launched_from_visual_studio, "launch contract should detect --from-vs");
    expect(result.request.read_only, "launch contract should detect --read-only");
    expect(result.output_json, "launch contract should detect --json");
    expect(result.request.apply_property_update, "launch contract should detect --set-property");
    expect(result.request.record_index == 3U, "launch contract should parse the record index");
    expect(result.request.property_name == "Left", "launch contract should capture the property name");
    expect(result.request.property_value == "25", "launch contract should capture the property value");
    expect(result.request.line == 25U, "launch contract should parse the line value");
    expect(result.request.column == 7U, "launch contract should parse the column value");
    expect(result.request.symbol == "cmdSave.Click", "launch contract should parse the symbol");
}

void test_parse_launch_arguments_rejects_unknown_switch() {
    const auto result = copperfin::studio::parse_launch_arguments({"--mystery"});
    expect(!result.ok, "launch contract should reject unknown switches");
}

void test_open_document_infers_form_sidecar() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_tests";
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "customer.scx";
    const fs::path sidecar_path = temp_dir / "customer.sct";

    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo-sidecar";
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .symbol = "form1",
        .line = 10U,
        .column = 2U,
        .launched_from_visual_studio = true,
        .read_only = false
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should succeed for a valid synthetic SCX file");
    expect(result.document.kind == copperfin::studio::StudioAssetKind::form, "SCX should map to a form document");
    expect(result.document.display_name == "customer.scx", "document display name should use the file name");
    expect(result.document.has_sidecar, "open_document should detect the SCT sidecar");
    expect(result.document.sidecar_path == sidecar_path.string(), "open_document should infer the SCT sidecar path");
    expect(result.document.launched_from_visual_studio, "launch metadata should flow into the Studio document");
    expect(result.document.inspection.header_available, "inspection metadata should be attached to the document");

    const auto objects = copperfin::studio::build_object_snapshot(result.document);
    expect(objects.empty(), "header-only synthetic SCX should not produce object snapshots without parsed records");

    std::error_code ignored;
    fs::remove(form_path, ignored);
    fs::remove(sidecar_path, ignored);
    fs::remove(temp_dir, ignored);
}

void test_open_document_preserves_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "missing_sidecar.scx";
    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .launched_from_visual_studio = false,
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for readable assets that carry validation findings");
    expect(
        result.document.inspection.has_validation_issues(),
        "Studio documents should retain validation findings from asset inspection");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "memo.sidecar_missing";
            }),
        "Studio documents should expose the missing-sidecar validation finding");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_memo_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_memo_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "payload_truncated.scx";
    const fs::path sidecar_path = temp_dir / "payload_truncated.sct";

    {
        std::vector<std::uint8_t> table_bytes(115U, 0U);
        table_bytes[0] = 0x30U;
        table_bytes[1] = 126U;
        table_bytes[2] = 4U;
        table_bytes[3] = 11U;
        table_bytes[4] = 0x01U;
        table_bytes[8] = 97U;
        table_bytes[10] = 18U;
        table_bytes[11] = 0U;
        table_bytes[28] = 0x00U;
        table_bytes[29] = 0x03U;
        table_bytes[32] = 'O';
        table_bytes[33] = 'B';
        table_bytes[34] = 'J';
        table_bytes[35] = 'N';
        table_bytes[36] = 'A';
        table_bytes[37] = 'M';
        table_bytes[38] = 'E';
        table_bytes[43] = 'C';
        table_bytes[44] = 1U;
        table_bytes[48] = 12U;
        table_bytes[64] = 'P';
        table_bytes[65] = 'R';
        table_bytes[66] = 'O';
        table_bytes[67] = 'P';
        table_bytes[68] = 'E';
        table_bytes[69] = 'R';
        table_bytes[70] = 'T';
        table_bytes[71] = 'I';
        table_bytes[72] = 'E';
        table_bytes[73] = 'S';
        table_bytes[75] = 'M';
        table_bytes[76] = 13U;
        table_bytes[80] = 4U;
        table_bytes[96] = 0x0DU;
        table_bytes[97] = 0x20U;
        table_bytes[98] = 't';
        table_bytes[99] = 'x';
        table_bytes[100] = 't';
        table_bytes[101] = 'T';
        table_bytes[102] = 'i';
        table_bytes[103] = 't';
        table_bytes[104] = 'l';
        table_bytes[105] = 'e';
        table_bytes[110] = 0x01U;
        table_bytes[114] = 0x1AU;

        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        memo_bytes[3] = 2U;
        memo_bytes[6] = 0x02U;
        memo_bytes[7] = 0x00U;
        memo_bytes[512U + 3U] = 1U;
        memo_bytes[512U + 4U] = 0x00U;
        memo_bytes[512U + 5U] = 0x00U;
        memo_bytes[512U + 6U] = 0x03U;
        memo_bytes[512U + 7U] = 0x84U;
        std::ofstream output(sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for forms with truncated memo payloads");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "memo.payload_truncated";
            }),
        "Studio documents should preserve memo payload validation findings");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_dbf_descriptor_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_descriptor_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "bad_fields.scx";
    {
        std::vector<std::uint8_t> bytes(129U, 0U);
        bytes[0] = 0x30U;
        bytes[1] = 126U;
        bytes[2] = 4U;
        bytes[3] = 11U;
        bytes[4] = 0x01U;
        bytes[8] = 97U;
        bytes[10] = 17U;
        bytes[11] = 0U;
        bytes[28] = 0x00U;
        bytes[29] = 0x03U;
        bytes[32] = '1';
        bytes[33] = '2';
        bytes[34] = '3';
        bytes[35] = 'B';
        bytes[36] = 'A';
        bytes[37] = 'D';
        bytes[38] = 'N';
        bytes[39] = 'A';
        bytes[40] = 'M';
        bytes[41] = 'E';
        bytes[43] = 'C';
        bytes[44] = 1U;
        bytes[48] = 8U;
        bytes[64] = '1';
        bytes[65] = '2';
        bytes[66] = '3';
        bytes[67] = 'B';
        bytes[68] = 'A';
        bytes[69] = 'D';
        bytes[70] = 'N';
        bytes[71] = 'A';
        bytes[72] = 'M';
        bytes[73] = 'E';
        bytes[75] = 'C';
        bytes[76] = 9U;
        bytes[80] = 8U;
        bytes[96] = 0x0DU;
        bytes[97] = 0x20U;
        bytes[128] = 0x1AU;

        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for assets with DBF descriptor validation findings");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "dbf.field_name_duplicate";
            }),
        "Studio documents should preserve DBF descriptor validation findings");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace

int main() {
    test_parse_launch_arguments();
    test_parse_launch_arguments_rejects_unknown_switch();
    test_open_document_infers_form_sidecar();
    test_open_document_preserves_validation_findings();
    test_open_document_preserves_memo_validation_findings();
    test_open_document_preserves_dbf_descriptor_validation_findings();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
