#include "copperfin/studio/document_model.h"
#include "copperfin/studio/vs_launch_contract.h"

#include <algorithm>
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
        "--symbol", "cmdSave.Click",
        "--undo-mode", "command",
        "--undo-label", "Bulk Undo"
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
    expect(result.request.undo_mode == copperfin::studio::StudioUndoMode::command, "launch contract should parse the undo mode");
    expect(result.request.undo_label == "Bulk Undo", "launch contract should parse the undo label");
}

void test_parse_launch_arguments_rejects_unknown_switch() {
    const auto result = copperfin::studio::parse_launch_arguments({"--mystery"});
    expect(!result.ok, "launch contract should reject unknown switches");
}

void test_parse_launch_arguments_rejects_unknown_undo_mode() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--undo-mode", "mystery"
    });
    expect(!result.ok, "launch contract should reject unknown undo modes");
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

void test_object_snapshot_preserves_empty_and_null_design_fields() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\customer.scx)";
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 7U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'C', .is_null = false, .display_value = "cmdSave"},
                {.field_name = "OBJTYPE", .field_type = 'N', .is_null = false, .display_value = "8.000"},
                {.field_name = "OBJCODE", .field_type = 'N', .is_null = false, .display_value = "1.000"},
                {.field_name = "PLATFORM", .field_type = 'C', .is_null = false, .display_value = "WINDOWS"},
                {.field_name = "PARENT", .field_type = 'C', .is_null = false, .display_value = "frmCustomer"},
                {.field_name = "HELP", .field_type = 'M', .is_null = true, .display_value = ""},
                {.field_name = "TAG", .field_type = 'M', .is_null = false, .display_value = ""},
                {.field_name = "PROPERTIES", .field_type = 'M', .is_null = false, .display_value = "Caption = Save"},
                {.field_name = "UNIQUEID", .field_type = 'C', .is_null = false, .display_value = "cmd-save-1"},
                {.field_name = "CLASS", .field_type = 'C', .is_null = false, .display_value = "commandbutton"},
                {.field_name = "BASECLASS", .field_type = 'C', .is_null = false, .display_value = "commandbutton"}
            }
        }
    };

    const auto objects = copperfin::studio::build_object_snapshot(document);
    expect(objects.size() == 1U, "#658: form design snapshot should include the parsed record");
    if (!objects.empty()) {
        expect(objects[0].objtype_code == 8, "#667: object snapshots should expose raw OBJTYPE metadata");
        expect(objects[0].objtype_field_index == 1U, "#671: raw OBJTYPE metadata should retain DBF field provenance");
        expect(objects[0].objcode_code == 1, "#667: object snapshots should expose raw OBJCODE metadata");
        expect(objects[0].objcode_field_index == 2U, "#671: raw OBJCODE metadata should retain DBF field provenance");
        expect(objects[0].platform == "WINDOWS", "#667: object snapshots should expose raw PLATFORM metadata");
        expect(objects[0].platform_field_index == 3U, "#671: raw PLATFORM metadata should retain DBF field provenance");
        expect(objects[0].object_name == "cmdSave", "#660: object snapshots should expose the design object name");
        expect(objects[0].object_name_field_index == 0U, "#672: object name metadata should retain DBF field provenance");
        expect(objects[0].unique_id == "cmd-save-1", "#660: object snapshots should expose stable UNIQUEID metadata");
        expect(objects[0].unique_id_field_index == 8U, "#672: UNIQUEID metadata should retain DBF field provenance");
        expect(objects[0].parent_name == "frmCustomer", "#660: object snapshots should expose parent hierarchy metadata");
        expect(objects[0].parent_name_field_index == 4U, "#672: parent hierarchy metadata should retain DBF field provenance");
        expect(objects[0].class_name == "commandbutton", "#660: object snapshots should expose CLASS metadata");
        expect(objects[0].class_name_field_index == 9U, "#672: CLASS metadata should retain DBF field provenance");
        expect(objects[0].baseclass_name == "commandbutton", "#660: object snapshots should expose BASECLASS metadata");
        expect(objects[0].baseclass_name_field_index == 10U, "#672: BASECLASS metadata should retain DBF field provenance");
        const auto parent = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "PARENT";
        });
        const auto tag = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "TAG";
        });
        const auto help = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "HELP";
        });
        const auto caption = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "Caption";
        });

        expect(parent != objects[0].properties.end(), "#660: parent design field should stay in object snapshots");
        if (parent != objects[0].properties.end()) {
            expect(parent->value == "frmCustomer", "#660: parent field should remain available as direct property metadata");
            expect(parent->field_index == 4U, "#659: direct design fields should preserve their DBF field ordinal");
            expect(!parent->derived_from_property_blob, "#659: direct DBF fields should not be marked blob-derived");
        }
        expect(help != objects[0].properties.end(), "#658: null design fields should stay in object snapshots");
        if (help != objects[0].properties.end()) {
            expect(help->is_null, "#658: null design field metadata should stay attached");
            expect(help->field_index == 5U, "#659: null direct fields should preserve their DBF field ordinal");
        }
        expect(tag != objects[0].properties.end(), "#658: empty memo-backed design fields should stay in object snapshots");
        if (tag != objects[0].properties.end()) {
            expect(tag->value.empty(), "#658: empty design fields should preserve their empty value");
            expect(tag->field_index == 6U, "#659: empty direct fields should preserve their DBF field ordinal");
        }
        expect(caption != objects[0].properties.end(), "#658: visual property blob expansion should still work");
        if (caption != objects[0].properties.end()) {
            expect(caption->field_index == 7U, "#659: blob-derived properties should retain the source PROPERTIES field ordinal");
            expect(caption->derived_from_property_blob, "#659: blob-derived properties should expose their provenance");
        }
    }
}

void test_menu_object_snapshot_preserves_normalized_menu_metadata() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\mainmenu.mnx)";
    document.kind = copperfin::studio::StudioAssetKind::menu;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Customer"},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "MAIN"},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "DO FORM customer"},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "Open customer maintenance"},
                {.field_name = "OBJTYPE", .field_type = 'N', .is_null = false, .display_value = "3.000"},
                {.field_name = "OBJCODE", .field_type = 'N', .is_null = false, .display_value = "7.000"}
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "TOOLS"},
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Tools"},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "DO tools"},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "tools_menu"},
                {.field_name = "PARENTID", .field_type = 'C', .is_null = false, .display_value = "main_menu"}
            }
        }
    };

    const auto objects = copperfin::studio::build_object_snapshot(document);
    expect(objects.size() == 2U, "#668: menu snapshot should include parsed menu records");
    if (objects.size() >= 1U) {
        expect(objects[0].menu_prompt == "Customer", "#668: menu snapshots should expose PROMPT metadata");
        expect(objects[0].menu_prompt_field_index == 0U, "#669: menu PROMPT metadata should retain DBF field provenance");
        expect(objects[0].menu_level_name == "MAIN", "#668: menu snapshots should expose LEVELNAME metadata");
        expect(objects[0].menu_level_name_field_index == 1U, "#669: menu LEVELNAME metadata should retain DBF field provenance");
        expect(objects[0].menu_command == "DO FORM customer", "#668: menu snapshots should expose COMMAND metadata");
        expect(objects[0].menu_command_field_index == 2U, "#669: menu COMMAND metadata should retain DBF field provenance");
        expect(objects[0].menu_message == "Open customer maintenance", "#668: menu snapshots should expose MESSAGE metadata");
        expect(objects[0].menu_message_field_index == 3U, "#669: menu MESSAGE metadata should retain DBF field provenance");
        expect(objects[0].title == "Customer", "#668: menu prompt should continue to drive friendly title fallback");
        expect(objects[0].subtitle == "MAIN", "#668: menu level name should continue to drive friendly subtitle fallback");
        expect(objects[0].objtype_code == 3, "#668: menu snapshots should retain raw OBJTYPE metadata");
        expect(objects[0].objcode_code == 7, "#668: menu snapshots should retain raw OBJCODE metadata");
    }
    if (objects.size() >= 2U) {
        expect(objects[1].menu_prompt == "Tools", "#668: menu snapshots should expose PROMPT metadata when it is not field zero");
        expect(objects[1].menu_prompt_field_index == 1U, "#670: present menu fields should keep their actual DBF ordinal");
        expect(objects[1].object_name == "tools_menu", "#672: object name metadata should fall back to NAME");
        expect(objects[1].object_name_field_index == 3U, "#672: object name fallback should retain selected NAME field provenance");
        expect(objects[1].parent_name == "main_menu", "#672: parent metadata should fall back to PARENTID");
        expect(objects[1].parent_name_field_index == 4U, "#672: parent fallback should retain selected PARENTID field provenance");
        expect(objects[1].unique_id_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing UNIQUEID provenance should use the object missing-field sentinel");
        expect(objects[1].class_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing CLASS provenance should use the object missing-field sentinel");
        expect(objects[1].baseclass_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing BASECLASS provenance should use the object missing-field sentinel");
        expect(objects[1].menu_message.empty(), "#670: missing menu MESSAGE values should remain empty");
        expect(objects[1].menu_message_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#670: missing menu MESSAGE provenance should not masquerade as field zero");
        expect(objects[1].objtype_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing OBJTYPE provenance should use the object missing-field sentinel");
        expect(objects[1].objcode_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing OBJCODE provenance should use the object missing-field sentinel");
        expect(objects[1].platform_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing PLATFORM provenance should use the object missing-field sentinel");
    }
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

void test_open_document_includes_prg_static_diagnostics() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_prg_diagnostics";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path program_path = temp_dir / "flagged.prg";
    {
        std::ofstream output(program_path, std::ios::binary);
        output << "DO WHILE .T.\n";
        output << "x = 1\n";
        output << "ENDDO\n";
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = program_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should succeed for a PRG file");
    expect(result.document.kind == copperfin::studio::StudioAssetKind::program, "PRG should map to a program document");
    expect(!result.document.static_diagnostics.empty(), "Studio documents should include PRG static diagnostics");
    expect(
        std::any_of(
            result.document.static_diagnostics.begin(),
            result.document.static_diagnostics.end(),
            [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
                return diagnostic.code == "PRG1001";
            }),
        "Studio documents should surface analyzer diagnostics for PRG files");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace

int main() {
    test_parse_launch_arguments();
    test_parse_launch_arguments_rejects_unknown_switch();
    test_parse_launch_arguments_rejects_unknown_undo_mode();
    test_open_document_infers_form_sidecar();
    test_object_snapshot_preserves_empty_and_null_design_fields();
    test_menu_object_snapshot_preserves_normalized_menu_metadata();
    test_open_document_preserves_validation_findings();
    test_open_document_preserves_memo_validation_findings();
    test_open_document_preserves_dbf_descriptor_validation_findings();
    test_open_document_includes_prg_static_diagnostics();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
