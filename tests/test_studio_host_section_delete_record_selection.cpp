// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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
    expect(create_result.ok, "#2791: section delete layout fixture should be created");
}

void run_section_delete_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_layout_reorder_json(asset_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--delete-object",
            "--record", "1",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host " << label << " section delete stdout:\n" << delete_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " section delete stderr:\n" << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect(dbf_record_deleted(asset_path, 1U), issue_prefix + " should mark the section record deleted");
    if (asset_path.extension() == ".lbx") {
        expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(
        delete_process.stdout_text,
        "\"sectionCount\": 0,\n      \"deletedSectionCount\": 1",
        issue_prefix + " should preserve top-level live and deleted section counts");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"detail_1\"",
            "\"idFieldIndex\": null",
            "\"idMemoBlockNumber\": 0",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"objectCount\": 3",
            "\"deletedObjectCount\": 0"
        },
        issue_prefix + " should preserve deleted-section identity, provenance, and object counts");
    expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve retained live-object preview availability after section delete");
    expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 100",
                    issue_prefix + " should preserve retained live-object left bounds after section delete");
    expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 2600",
                    issue_prefix + " should preserve retained live-object top bounds after section delete");
    expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 150",
                    issue_prefix + " should preserve retained live-object right bounds after section delete");
    expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 2800",
                    issue_prefix + " should preserve retained live-object bottom bounds after section delete");
    expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 50",
                    issue_prefix + " should preserve retained live-object preview width after section delete");
    expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 200",
                    issue_prefix + " should preserve retained live-object preview height after section delete");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should expose deleted preview availability after section delete");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    issue_prefix + " should preserve deleted preview left bounds after section delete");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                    issue_prefix + " should preserve deleted preview top bounds after section delete");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                    issue_prefix + " should preserve deleted preview right bounds after section delete");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                    issue_prefix + " should preserve deleted preview bottom bounds after section delete");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                    issue_prefix + " should preserve deleted preview widths after section delete");
    expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                    issue_prefix + " should preserve deleted preview heights after section delete");
    expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    issue_prefix + " should advertise selected-section availability");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    issue_prefix + " should expose section selection kind");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"detail_1\"",
            "\"idFieldIndex\": null",
            "\"idMemoBlockNumber\": 0",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"objectCount\": 3",
            "\"deletedObjectCount\": 0"
        },
        issue_prefix + " should preserve selected-section identity, provenance, and object counts");
    expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 3",
                    issue_prefix + " should preserve three placed objects after section delete");
    expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 0",
                    issue_prefix + " should preserve zero unplaced objects after section delete");
    expect_contains(delete_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    issue_prefix + " should preserve deleted-section object membership");
}

void test_studio_host_json_preserves_section_delete_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_section_delete_record_selection_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_section_delete_case(
        studio_host_path,
        temp_root,
        "summary.frx",
        "report",
        "#2791: record-selected report section delete");
    run_section_delete_case(
        studio_host_path,
        temp_root,
        "mailing.lbx",
        "label",
        "#2791: record-selected label section delete");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_section_delete_record_selection <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_section_delete_record_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
