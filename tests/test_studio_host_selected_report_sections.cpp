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
    expect(create_result.ok, "#2830: layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2830: layout fixture should mark deleted layout objects");
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
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", ""},
        {"8", "0", "left.value", "100", "2600", "50", "200", "left-field-guid"},
        {"8", "0", "middle.value", "100", "2600", "50", "200", "middle-field-guid"},
        {"8", "0", "right.value", "100", "2600", "50", "200", "right-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2830: reorder fixture should be created");
}

void write_synthetic_report_table_for_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#2830: deleted section fixture should mark the section deleted");
}

void test_studio_host_json_preserves_selected_report_sections(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_report_sections_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(report_path), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host selected report section stdout:\n" << section_process.stdout_text << "\n";
        std::cerr << "studio host selected report section stderr:\n" << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2830: selected report section JSON smoke should exit successfully");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2830: report section selections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2830: report section selections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2830: report section selections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"selectedReportSection\": {",
                    "#2830: report section selections should expose selected-section JSON");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2830: selected report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2830: selected report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2830: selected report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#2830: selected report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#2830: selected report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#2830: selected report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#2830: selected report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#2830: selected report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#2830: selected report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#2830: selected report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#2830: selected report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#2830: selected report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#2830: selected report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#2830: selected report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2830: selected report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2830: selected report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2830: selected report sections should not advertise containing-object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2830: selected report sections should serialize null containing-object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2830: selected report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2830: selected report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"id\": \"page_header_1\"",
                    "#2830: selected report section JSON should expose the selected section id");
    expect_contains(section_process.stdout_text, "\"bandKind\": \"page_header\"",
                    "#2830: selected report section JSON should expose the selected band kind");
    expect_contains(section_process.stdout_text, "\"recordIndex\": 1",
                    "#2830: selected report section JSON should expose the selected section record index");
    expect_contains(section_process.stdout_text, "\"sectionIndex\": 0",
                    "#2830: selected report section JSON should expose section order");
    expect_contains(
        section_process.stdout_text,
        "\"sectionCount\": 2,\n      \"deletedSectionCount\": 0",
        "#2830: selected report section JSON should expose top-level live and deleted section counts");
    expect_contains(section_process.stdout_text, "\"objectCode\": 1",
                    "#2830: selected report section JSON should preserve raw section object codes");
    expect_contains(section_process.stdout_text, "\"bottom\": 2000",
                    "#2830: selected report section JSON should expose section bottom-edge coordinates");
    expect_contains(section_process.stdout_text, "\"objectCount\": 1",
                    "#2830: selected report section JSON should preserve selected section object counts");
    expect_contains(section_process.stdout_text, "\"objectKind\": \"label\"",
                    "#2830: selected report section JSON should include selected section objects");

    const fs::path deleted_section_path = temp_root / "deleted_section.frx";
    write_synthetic_report_table_for_deleted_section_json(deleted_section_path);
    const auto deleted_section_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(deleted_section_path), "--record", "1", "--json"},
        temp_root);

    if (deleted_section_process.exit_code != 0) {
        std::cerr << "studio host selected deleted report section stdout:\n"
                  << deleted_section_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted report section stderr:\n"
                  << deleted_section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_section_process.exit_code == 0,
           "#2830: selected deleted report section JSON should exit successfully");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2830: deleted report section selections should advertise selected-section availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2830: deleted report section selections should advertise report-selection availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2830: deleted report section selections should expose section selection kind");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2830: selected deleted report sections should not advertise selected-object availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2830: selected deleted report sections should serialize null selected objects");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2830: selected deleted report sections should not advertise containing-object-section availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2830: selected deleted report sections should serialize null containing-object sections");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2830: selected deleted report sections should not advertise selected-settings availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2830: selected deleted report sections should serialize null selected settings");
    expect_contains(
        deleted_section_process.stdout_text,
        "\"sectionCount\": 0,\n      \"deletedSectionCount\": 1",
        "#2830: deleted selected report section JSON should expose top-level live and deleted section counts");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2830: deleted selected report section JSON should preserve retained live-object preview availability");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsLeft\": 100",
                    "#2830: deleted selected report section JSON should preserve retained live-object left bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsTop\": 2600",
                    "#2830: deleted selected report section JSON should preserve retained live-object top bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsRight\": 150",
                    "#2830: deleted selected report section JSON should preserve retained live-object right bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsBottom\": 2800",
                    "#2830: deleted selected report section JSON should preserve retained live-object bottom bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsWidth\": 50",
                    "#2830: deleted selected report section JSON should preserve retained live-object preview width");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsHeight\": 200",
                    "#2830: deleted selected report section JSON should preserve retained live-object preview height");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#2830: deleted selected report section JSON should expose deleted preview bounds availability");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2830: deleted selected report section JSON should expose deleted section preview left bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                    "#2830: deleted selected report section JSON should expose deleted section preview top bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                    "#2830: deleted selected report section JSON should expose deleted section preview right bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                    "#2830: deleted selected report section JSON should expose deleted section preview bottom bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                    "#2830: deleted selected report section JSON should expose deleted section preview width");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                    "#2830: deleted selected report section JSON should expose deleted section preview height");
    expect_contains(deleted_section_process.stdout_text, "\"placedObjectCount\": 3",
                    "#2830: deleted selected report section JSON should preserve live object counts outside section totals");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPlacedObjectCount\": 0",
                    "#2830: deleted selected report section JSON should not fabricate deleted placed object counts");
    expect_contains(deleted_section_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    "#2830: deleted selected report section JSON should not fabricate deleted unplaced object counts");
    expect_contains(deleted_section_process.stdout_text, "\"sectionKindCount\": 0",
                    "#2830: deleted selected report section JSON should not fabricate live section band-kind buckets");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionKindCount\": 1",
                    "#2830: deleted selected report section JSON should summarize deleted section band-kind buckets");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionKindCounts\": [\n        {\"kind\": \"detail\", \"count\": 1}\n      ]",
                    "#2830: deleted selected report section JSON should count deleted detail sections");
    expect_contains(deleted_section_process.stdout_text, "\"sectionHeightTotal\": 0",
                    "#2830: deleted selected report section JSON should not fabricate live section heights");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionHeightTotal\": 5000",
                    "#2830: deleted selected report section JSON should summarize deleted section heights");
    expect_contains_in_order(
        deleted_section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0"
        },
        "#2830: deleted report section selections should expose deleted selected-section metadata");

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", copperfin::test_support::path_to_utf8_string(report_path), "--record", "3", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host selected report object stdout:\n" << object_process.stdout_text << "\n";
        std::cerr << "studio host selected report object stderr:\n" << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0,
           "#2830: selected report object JSON smoke should exit successfully");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#2830: non-section report selections should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#2830: non-section report selections should serialize null selected sections");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_report_sections <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_report_sections(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
