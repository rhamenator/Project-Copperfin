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

void expect_not_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) == std::string::npos, message);
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

void expect_empty_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": false",
                    prefix + " should not fabricate live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve zero live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve zero live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve zero live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 0",
                    prefix + " should preserve zero live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve zero live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 0",
                    prefix + " should preserve zero live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": false",
                    prefix + " should not fabricate deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
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

void write_synthetic_report_table_for_column_setup_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "COLS=2\nCOLWIDTH=3600\nCOLSPACING=120", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2790: stable column-setup settings fixture should be created");
}

void write_synthetic_report_table_for_deleted_column_setup_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_setup_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#2790: stable deleted column-setup settings fixture should mark settings deleted");
}

void write_synthetic_report_table_for_stable_column_setup_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_setup_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#2790: stable column-setup fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#2790: stable column-setup fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_column_setup_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_column_setup_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#2790: stable deleted column-setup fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#2790: stable deleted column-setup fixture should preserve the deleted settings state");
}

void run_column_setup_update_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    const std::string& unique_id,
    const std::string& updated_settings,
    const std::string& expected_count,
    const std::string& expected_width,
    const std::string& expected_spacing,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_stable_deleted_column_setup_json(asset_path);
    } else {
        write_synthetic_report_table_for_stable_column_setup_json(asset_path);
    }

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--set-property",
            "--unique-id", unique_id,
            "--property-name", "EXPR",
            "--property-value", updated_settings,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable column setup update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable column setup update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .property_name = "EXPR"
    });
    expect(expr_property.ok && expr_property.exists && expr_property.value == updated_settings,
           issue_prefix + " update should persist the EXPR memo field");
    expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " update should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(update_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " update should retain label identity");
    }
    if (!deleted) {
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2037: stable-selected report/label column setup update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        issue_prefix + " update should not fabricate page setup availability");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        issue_prefix + " update should preserve column setup availability");
        expect_contains(update_process.stdout_text, "\"columnCount\": " + expected_count,
                        issue_prefix + " update should refresh column counts");
        expect_contains(update_process.stdout_text, "\"columnWidth\": " + expected_width,
                        issue_prefix + " update should refresh column widths");
        expect_contains(update_process.stdout_text, "\"columnSpacing\": " + expected_spacing,
                        issue_prefix + " update should refresh column spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 3",
                        issue_prefix + " update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"value\": \"" + expected_count + "\"",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"value\": \"" + expected_width + "\"",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"value\": \"" + expected_spacing + "\""
            },
            issue_prefix + " update should refresh selected setting provenance");
    } else {
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2037: stable-selected deleted report/label column setup update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        issue_prefix + " update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        issue_prefix + " update should expose effective deleted-root column setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 3",
                        issue_prefix + " update should refresh deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"value\": \"" + expected_count + "\"",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"value\": \"" + expected_width + "\"",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"value\": \"" + expected_spacing + "\""
            },
            issue_prefix + " update should refresh deleted setting provenance");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"value\": \"" + expected_count + "\"",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"value\": \"" + expected_width + "\"",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"value\": \"" + expected_spacing + "\""
            },
            issue_prefix + " update should refresh selected deleted settings");
    }
}

void run_column_setup_clear_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    const std::string& unique_id,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_stable_deleted_column_setup_json(asset_path);
    } else {
        write_synthetic_report_table_for_stable_column_setup_json(asset_path);
    }

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--clear-property",
            "--unique-id", unique_id,
            "--property-name", "EXPR",
            "--json"
        },
        temp_root);

    if (clear_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable column setup clear " << extension << " stdout:\n"
                  << clear_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable column setup clear " << extension << " stderr:\n"
                  << clear_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(clear_process.exit_code == 0, issue_prefix + " clear should exit successfully");
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .property_name = "EXPR"
    });
    expect(expr_property.ok && expr_property.exists && expr_property.direct_field,
           issue_prefix + " clear should preserve the direct EXPR field carrier");
    expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " clear should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " clear should retain label identity");
    }
    if (!deleted) {
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2037: stable-selected report/label column setup clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        issue_prefix + " clear should not fabricate page setup availability");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": false",
                        issue_prefix + " clear should clear column setup availability");
        expect_contains(clear_process.stdout_text, "\"columnCountAvailable\": false",
                        issue_prefix + " clear should clear column-count availability");
        expect_contains(clear_process.stdout_text, "\"columnWidthAvailable\": false",
                        issue_prefix + " clear should clear column-width availability");
        expect_contains(clear_process.stdout_text, "\"columnSpacingAvailable\": false",
                        issue_prefix + " clear should clear column-spacing availability");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " clear should remove memo-derived settings from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        issue_prefix + " clear should clear selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettings\": null",
                        issue_prefix + " clear should clear selected settings");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        issue_prefix + " clear should clear settings selection kind");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"COLS\"",
                            issue_prefix + " clear should remove column-count settings");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"COLWIDTH\"",
                            issue_prefix + " clear should remove column-width settings");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"COLSPACING\"",
                            issue_prefix + " clear should remove column-spacing settings");
    } else {
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2037: stable-selected deleted report/label column setup clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        issue_prefix + " clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": false",
                        issue_prefix + " clear should not fabricate live column setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 0",
                        issue_prefix + " clear should remove deleted settings");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        issue_prefix + " clear should clear selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettings\": null",
                        issue_prefix + " clear should clear selected deleted settings");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        issue_prefix + " clear should clear settings selection kind");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"COLS\"",
                            issue_prefix + " clear should remove deleted column-count settings");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"COLWIDTH\"",
                            issue_prefix + " clear should remove deleted column-width settings");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"COLSPACING\"",
                            issue_prefix + " clear should remove deleted column-spacing settings");
    }
}

void test_studio_host_json_preserves_column_setup_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_column_setup_stable_settings_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_column_setup_update_case(
        studio_host_path,
        temp_root,
        "column_setup_stable",
        ".frx",
        "settings-guid",
        "COLS=3\nCOLWIDTH=2400\nCOLSPACING=180",
        "3",
        "2400",
        "180",
        false,
        "report",
        "#2790: report/label stable-selected column-setup settings success");
    run_column_setup_update_case(
        studio_host_path,
        temp_root,
        "column_setup_stable",
        ".lbx",
        "settings-guid",
        "COLS=3\nCOLWIDTH=2400\nCOLSPACING=180",
        "3",
        "2400",
        "180",
        false,
        "label",
        "#2790: report/label stable-selected column-setup settings success");
    run_column_setup_clear_case(
        studio_host_path,
        temp_root,
        "column_setup_clear_stable",
        ".frx",
        "settings-guid",
        false,
        "report",
        "#2790: report/label stable-selected column-setup settings success");
    run_column_setup_clear_case(
        studio_host_path,
        temp_root,
        "column_setup_clear_stable",
        ".lbx",
        "settings-guid",
        false,
        "label",
        "#2790: report/label stable-selected column-setup settings success");
    run_column_setup_update_case(
        studio_host_path,
        temp_root,
        "deleted_column_setup_stable",
        ".frx",
        "deleted-settings-guid",
        "COLS=4\nCOLWIDTH=1800\nCOLSPACING=240",
        "4",
        "1800",
        "240",
        true,
        "report",
        "#2790: report/label stable-selected deleted column-setup settings success");
    run_column_setup_update_case(
        studio_host_path,
        temp_root,
        "deleted_column_setup_stable",
        ".lbx",
        "deleted-settings-guid",
        "COLS=4\nCOLWIDTH=1800\nCOLSPACING=240",
        "4",
        "1800",
        "240",
        true,
        "label",
        "#2790: report/label stable-selected deleted column-setup settings success");
    run_column_setup_clear_case(
        studio_host_path,
        temp_root,
        "deleted_column_setup_clear_stable",
        ".frx",
        "deleted-settings-guid",
        true,
        "report",
        "#2790: report/label stable-selected deleted column-setup settings success");
    run_column_setup_clear_case(
        studio_host_path,
        temp_root,
        "deleted_column_setup_clear_stable",
        ".lbx",
        "deleted-settings-guid",
        true,
        "label",
        "#2790: report/label stable-selected deleted column-setup settings success");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_column_setup_stable_settings <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_column_setup_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
