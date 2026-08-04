// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_table.h"
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

bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index) {
    const auto table_result =
        copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return false;
    }
    return table_result.table.records[record_index].deleted;
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
    expect(create_result.ok, "#2826: layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2826: layout fixture should mark deleted layout objects");
}

void run_orphaned_page_header_object_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_layout_json(asset_path);
    const auto delete_section_result =
        copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
    expect(delete_section_result.ok && dbf_record_deleted(asset_path, 1U),
           issue_prefix + " should mark the containing section deleted");

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(asset_path), "--unique-id", "label-guid", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected orphaned page-header object stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected orphaned page-header object stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    issue_prefix + " should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    issue_prefix + " should expose object selection kind");
    expect_contains(
        object_process.stdout_text,
        "\"sectionCount\": 1,\n      \"deletedSectionCount\": 1",
        issue_prefix + " should preserve top-level live and deleted section counts");
    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                    issue_prefix + " should preserve live object counts");
    expect_contains(object_process.stdout_text, "\"placedObjectCount\": 2",
                    issue_prefix + " should keep live objects inside deleted page-header sections placed");
    expect_contains(object_process.stdout_text, "\"unplacedObjectCount\": 1",
                    issue_prefix + " should not count deleted-section page-header objects as unplaced");
    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " should preserve deleted object counts");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 100",
                    issue_prefix + " should preserve retained page-header-object top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8000",
                    issue_prefix + " should preserve retained page-header-object preview height");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should expose deleted preview availability");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                    issue_prefix + " should expand deleted preview right bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2700",
                    issue_prefix + " should expand deleted preview widths");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2900",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    issue_prefix + " should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    issue_prefix + " should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    issue_prefix + " should advertise containing-section availability");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"objectCount\": 1"
        },
        issue_prefix + " should expose deleted containing-section JSON");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 4",
            "\"deleted\": false",
            "\"containingSectionId\": \"page_header_1\"",
            "\"containingSectionRecordIndex\": 1",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 450",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Invoice\\\"\"",
            "\"expressionFieldIndex\": 2"
        },
        issue_prefix + " should expose selected object metadata with deleted-section membership");
    expect_contains(object_process.stdout_text, "\"left\": 900",
                    issue_prefix + " should expose selected-object left bounds");
    expect_contains(object_process.stdout_text, "\"top\": 100",
                    issue_prefix + " should expose selected-object top bounds");
    expect_contains(object_process.stdout_text, "\"width\": 1800",
                    issue_prefix + " should expose selected-object widths");
    expect_contains(object_process.stdout_text, "\"right\": 2700",
                    issue_prefix + " should expose selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"height\": 350",
                    issue_prefix + " should expose selected-object heights");
    expect_contains(object_process.stdout_text, "\"bottom\": 450",
                    issue_prefix + " should expose selected-object bottom bounds");
    expect_contains(
        object_process.stdout_text,
        "\"name\": \"EXPR\", \"recordIndex\": 4, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 4, \"value\": \"\\\"Invoice\\\"\"",
        issue_prefix + " should expose selected-object expression provenance");
}

void test_studio_host_json_preserves_selected_page_header_objects_orphaned_by_deleted_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_orphaned_page_header_objects_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_orphaned_page_header_object_selection(
        studio_host_path,
        temp_root,
        "selected_orphaned_page_header_object_stable.frx",
        "report",
        "#2826: orphaned live report page-header object selection");
    run_orphaned_page_header_object_selection(
        studio_host_path,
        temp_root,
        "selected_orphaned_page_header_object_stable.lbx",
        "label",
        "#2826: orphaned live label page-header object selection");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_page_header_objects_orphaned <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_page_header_objects_orphaned_by_deleted_sections(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
