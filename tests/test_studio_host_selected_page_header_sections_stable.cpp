// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_locale_catalog_environment_support.h"
#include "test_process_capture_support.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;
using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;

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

void write_synthetic_report_table_for_layout_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "10", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2801: selected page-header layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2801: selected page-header layout fixture should mark deleted objects");
}

void run_live_page_header_section_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_layout_json(asset_path);

    const auto seed_identity = copperfin::vfp::update_visual_object_property({
        .path = asset_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "page-header-section-guid"
    });
    expect(seed_identity.ok, issue_prefix + " fixture should seed a stable id");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "page-header-section-guid", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected page-header section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected page-header section stderr:\n"
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
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 5200",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should preserve deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    issue_prefix + " should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    issue_prefix + " should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(
        section_process.stdout_text,
        "\"sectionCount\": 2,\n      \"deletedSectionCount\": 0",
        issue_prefix + " should preserve top-level live and deleted section counts");
    expect_contains(section_process.stdout_text, "\"liveObjectCount\": 3",
                    issue_prefix + " should preserve live object counts");
    expect_contains(section_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " should preserve deleted object counts");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"page-header-section-guid\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 2",
            "\"top\": 0",
            "\"height\": 2000",
            "\"bottom\": 2000",
            "\"objectCount\": 1"
        },
        issue_prefix + " should expose selected section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"objects\": [",
            "\"recordIndex\": 4",
            "\"deleted\": false",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Invoice\\\"\"",
            "\"left\": 900",
            "\"top\": 100",
            "\"right\": 2700",
            "\"bottom\": 450"
        },
        issue_prefix + " should expose section object membership");
}

void run_deleted_page_header_section_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_layout_json(asset_path);

    const auto seed_identity = copperfin::vfp::update_visual_object_property({
        .path = asset_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-page-header-guid"
    });
    expect(seed_identity.ok, issue_prefix + " fixture should seed a stable id");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
    expect(delete_result.ok && dbf_record_deleted(asset_path, 1U),
           issue_prefix + " fixture should mark the section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "deleted-page-header-guid", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected deleted page-header section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected deleted page-header section stderr:\n"
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
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 100",
                    issue_prefix + " should preserve retained page-header-object top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 5200",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 8000",
                    issue_prefix + " should preserve retained page-header-object preview height");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                    issue_prefix + " should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2700",
                    issue_prefix + " should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2900",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(
        section_process.stdout_text,
        "\"sectionCount\": 1,\n      \"deletedSectionCount\": 1",
        issue_prefix + " should preserve top-level live and deleted section counts");
    expect_contains(section_process.stdout_text, "\"liveObjectCount\": 3",
                    issue_prefix + " should preserve live object counts");
    expect_contains(section_process.stdout_text, "\"unplacedObjectCount\": 1",
                    issue_prefix + " should preserve unrelated unplaced object accounting");
    expect_contains(section_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " should preserve deleted object counts");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"deleted-page-header-guid\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 0",
            "\"height\": 2000",
            "\"bottom\": 2000"
        },
        issue_prefix + " should expose selected deleted-section metadata");
}

void test_studio_host_json_preserves_selected_page_header_sections_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_page_header_sections_stable_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_live_page_header_section_selection(
        studio_host_path,
        temp_root,
        "selected_page_header_section_stable.frx",
        "report",
        "#2801: stable live report page-header section selection");
    run_live_page_header_section_selection(
        studio_host_path,
        temp_root,
        "selected_page_header_section_stable.lbx",
        "label",
        "#2801: stable live label page-header section selection");
    run_deleted_page_header_section_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_page_header_section_stable.frx",
        "report",
        "#2801: stable deleted report page-header section selection");
    run_deleted_page_header_section_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_page_header_section_stable.lbx",
        "label",
        "#2801: stable deleted label page-header section selection");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_page_header_sections_stable <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_page_header_sections_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
