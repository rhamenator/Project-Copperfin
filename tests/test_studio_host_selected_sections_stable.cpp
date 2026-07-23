// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_locale_catalog_environment_support.h"
#include "test_process_capture_support.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) != std::string::npos, message);
}

void expect_contains_in_order(
    const std::string& text,
    const std::vector<std::string>& needles,
    const std::string& message) {
    std::size_t offset = 0U;
    for (const auto& needle : needles) {
        const std::size_t position = text.find(needle, offset);
        if (position == std::string::npos) {
            expect(false, message);
            return;
        }
        offset = position + needle.size();
    }
}

bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index) {
    const auto table_result =
        copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return false;
    }
    return table_result.table.records[record_index].deleted;
}

using ProcessResult = copperfin::test_support::CapturedProcessResult;

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    return copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(
            copperfin::test_support::path_from_utf8_string(executable_path),
            arguments,
            working_directory));
}

void write_synthetic_report_table_for_layout_reorder_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "PAGEBREAK", .type = 'L', .length = 1U},
        {.name = "COLBREAK", .type = 'L', .length = 1U},
        {.name = "RESETPAGE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", "", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", ".T.", ".F.", ".T."},
        {"8", "0", "left.value", "100", "2600", "50", "200", "left-field-guid", "", "", ""},
        {"8", "0", "middle.value", "100", "2600", "50", "200", "middle-field-guid", "", "", ""},
        {"8", "0", "right.value", "100", "2600", "50", "200", "right-field-guid", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2800: selected-sections layout fixture should be created");
}

void write_synthetic_report_table_for_stable_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "section-guid"
    });
    expect(unique_id_result.ok, "#2800: stable section fixture should seed a section unique id");
    expect(!dbf_record_deleted(report_path, 1U),
           "#2800: stable section fixture should preserve the live section state");
}

void write_synthetic_report_table_for_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#2800: deleted section fixture should mark section deleted");
}

void write_synthetic_report_table_for_stable_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_section_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-section-guid"
    });
    expect(unique_id_result.ok, "#2800: stable deleted section fixture should seed a deleted section unique id");
    expect(dbf_record_deleted(report_path, 1U),
           "#2800: stable deleted section fixture should preserve the deleted section state");
}

void run_live_section_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_section_json(asset_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "section-guid", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(section_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    issue_prefix + " should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    issue_prefix + " should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 2000",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 150",
                    issue_prefix + " should include selected section object right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 7000",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 150",
                    issue_prefix + " should include selected section object widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 5000",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    issue_prefix + " should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"pageBreak\": \"true\"",
                    issue_prefix + " should expose the invariant PAGEBREAK section value");
    expect_contains(section_process.stdout_text, "\"pageBreakFieldIndex\": 8",
                    issue_prefix + " should expose PAGEBREAK field provenance");
    expect_contains(section_process.stdout_text, "\"columnBreak\": \"false\"",
                    issue_prefix + " should expose the invariant COLBREAK section value");
    expect_contains(section_process.stdout_text, "\"resetPage\": \"true\"",
                    issue_prefix + " should expose the invariant RESETPAGE section value");
    expect_contains(
        section_process.stdout_text,
        "\"sectionCount\": 1,\n      \"deletedSectionCount\": 0",
        issue_prefix + " should preserve top-level live and deleted section counts");
    expect_contains(section_process.stdout_text, "\"unplacedObjectCount\": 0",
                    issue_prefix + " should preserve section object membership");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 1",
            "\"objectCount\": 3"
        },
        issue_prefix + " should expose selected live-section metadata");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--unique-id", "section-guid",
            "--property-name", "PAGEBREAK", "--property-value", "false",
            "--property-name", "COLBREAK", "--property-value", "true",
            "--property-name", "RESETPAGE", "--property-value", "false",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
           issue_prefix + " should round-trip section pagination flag edits through the host update path");
    const auto reopened_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "section-guid", "--json"},
        temp_root);
    expect(reopened_process.exit_code == 0,
           issue_prefix + " should reopen a section after pagination flag edits");
    expect_contains(reopened_process.stdout_text, "\"pageBreak\": \"false\"",
                    issue_prefix + " should persist PAGEBREAK edits");
    expect_contains(reopened_process.stdout_text, "\"columnBreak\": \"true\"",
                    issue_prefix + " should persist COLBREAK edits");
    expect_contains(reopened_process.stdout_text, "\"resetPage\": \"false\"",
                    issue_prefix + " should persist RESETPAGE edits");
}

void run_deleted_section_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_deleted_section_json(asset_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "deleted-section-guid", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected deleted section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected deleted section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(section_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    issue_prefix + " should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    issue_prefix + " should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve retained live-object preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 100",
                    issue_prefix + " should preserve retained live-object left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 2600",
                    issue_prefix + " should preserve retained live-object top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 150",
                    issue_prefix + " should preserve retained live-object right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 2800",
                    issue_prefix + " should preserve retained live-object bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 50",
                    issue_prefix + " should preserve retained live-object preview width");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 200",
                    issue_prefix + " should preserve retained live-object preview height");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    issue_prefix + " should preserve deleted section left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                    issue_prefix + " should preserve deleted section top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                    issue_prefix + " should preserve deleted section right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                    issue_prefix + " should preserve deleted section bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                    issue_prefix + " should preserve deleted section widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                    issue_prefix + " should preserve deleted section heights");
    expect_contains(
        section_process.stdout_text,
        "\"sectionCount\": 0,\n      \"deletedSectionCount\": 1",
        issue_prefix + " should preserve top-level live and deleted section counts");
    expect_contains(section_process.stdout_text, "\"unplacedObjectCount\": 0",
                    issue_prefix + " should preserve deleted-section object membership");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0"
        },
        issue_prefix + " should expose selected deleted-section metadata");
    expect_contains(section_process.stdout_text, "\"pageBreak\": \"true\"",
                    issue_prefix + " should expose deleted-section PAGEBREAK values");
    expect_contains(section_process.stdout_text, "\"columnBreak\": \"false\"",
                    issue_prefix + " should expose deleted-section COLBREAK values");
    expect_contains(section_process.stdout_text, "\"resetPage\": \"true\"",
                    issue_prefix + " should expose deleted-section RESETPAGE values");
}

void test_studio_host_json_preserves_selected_sections_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_sections_stable_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_live_section_selection(
        studio_host_path,
        temp_root,
        "selected_section_stable.frx",
        "report",
        "#2800: stable live report section selection");
    run_live_section_selection(
        studio_host_path,
        temp_root,
        "selected_section_stable.lbx",
        "label",
        "#2800: stable live label section selection");
    run_deleted_section_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_section_stable.frx",
        "report",
        "#2800: stable deleted report section selection");
    run_deleted_section_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_section_stable.lbx",
        "label",
        "#2800: stable deleted label section selection");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_sections_stable <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_sections_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
