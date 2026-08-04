// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_invalid_direct_column_setup_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "many", "wide?", "spaced?", "invalid-direct-live-column-settings-guid"},
        {"1", "53", "deleted-many", "deleted-wide?", "deleted-spaced?",
         "invalid-direct-deleted-column-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1734: synthetic report table with invalid direct column setup fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1734: synthetic report table should mark invalid direct column settings deleted");
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
    expect(create_result.ok, "#1518: synthetic report table for column setup JSON should be created");
}

void write_synthetic_report_table_for_column_setup_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "COLS", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "COLWIDTH=3600\nCOLSPACING=120", "2", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1538: synthetic report table for column setup field JSON should be created");
}

void write_synthetic_report_table_for_deleted_column_setup_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_setup_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1593: synthetic report table should mark column-count settings deleted");
}

void write_synthetic_report_table_for_stable_column_setup_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_setup_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1835: stable column-count fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1835: stable column-count fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_column_setup_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_column_setup_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1835: stable deleted column-count fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1835: stable deleted column-count fixture should preserve the deleted settings state");
}

void test_studio_host_json_exposes_report_layout_column_setup(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_column_setup_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "columns.frx";
    write_synthetic_report_table_for_column_setup_json(report_path);
    const auto report_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--json"},
        temp_root);

    if (report_process.exit_code != 0) {
        std::cerr << "studio host report column setup stdout:\n" << report_process.stdout_text << "\n";
        std::cerr << "studio host report column setup stderr:\n" << report_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(report_process.exit_code == 0,
           "#1518: report column setup JSON should exit successfully");
    expect_contains(report_process.stdout_text, "\"reportLayout\": {",
                    "#1518: report column setup JSON should expose report-layout JSON");
    expect_contains(report_process.stdout_text, "\"pageSetupAvailable\": false",
                    "#1518: report column setup JSON should not fabricate page setup availability");
    expect_contains(report_process.stdout_text, "\"columnSetupAvailable\": true",
                    "#1518: report column setup JSON should expose column setup availability");
    expect_contains(report_process.stdout_text, "\"columnCountAvailable\": true",
                    "#1518: report column setup JSON should expose column count availability");
    expect_contains(report_process.stdout_text, "\"columnCount\": 2",
                    "#1518: report column setup JSON should expose column counts");
    expect_contains(report_process.stdout_text, "\"columnWidthAvailable\": true",
                    "#1518: report column setup JSON should expose column width availability");
    expect_contains(report_process.stdout_text, "\"columnWidth\": 3600",
                    "#1518: report column setup JSON should expose column widths");
    expect_contains(report_process.stdout_text, "\"columnSpacingAvailable\": true",
                    "#1518: report column setup JSON should expose column spacing availability");
    expect_contains(report_process.stdout_text, "\"columnSpacing\": 120",
                    "#1518: report column setup JSON should expose column spacing");
    expect_contains(report_process.stdout_text, "\"previewBoundsAvailable\": false",
                    "#2017: report column setup JSON should not fabricate live preview availability");
    expect_contains(report_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2017: report column setup JSON should preserve zero live preview left bounds");
    expect_contains(report_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2017: report column setup JSON should preserve zero live preview top bounds");
    expect_contains(report_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2017: report column setup JSON should preserve zero live preview right bounds");
    expect_contains(report_process.stdout_text, "\"previewBoundsBottom\": 0",
                    "#2017: report column setup JSON should preserve zero live preview bottom bounds");
    expect_contains(report_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2017: report column setup JSON should preserve zero live preview widths");
    expect_contains(report_process.stdout_text, "\"previewBoundsHeight\": 0",
                    "#2017: report column setup JSON should preserve zero live preview heights");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2017: report column setup JSON should not fabricate deleted preview availability");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview left bounds");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview top bounds");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview right bounds");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview bottom bounds");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview widths");
    expect_contains(report_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                    "#2017: report column setup JSON should preserve zero deleted preview heights");
    expect_contains(report_process.stdout_text, "\"settingCount\": 3",
                    "#1518: report column setup JSON should preserve compact setting counts");
    expect_contains(report_process.stdout_text, "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"2\"",
                    "#1518: report column setup JSON should preserve column-count setting provenance");
    expect_contains(report_process.stdout_text, "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"3600\"",
                    "#1518: report column setup JSON should preserve column-width setting provenance");
    expect_contains(report_process.stdout_text, "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2, \"memoBlockNumber\": 1, \"value\": \"120\"",
                    "#1518: report column setup JSON should preserve column-spacing setting provenance");

    const fs::path label_path = temp_root / "columns.lbx";
    write_synthetic_report_table_for_column_setup_json(label_path);
    const auto label_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--json"},
        temp_root);

    if (label_process.exit_code != 0) {
        std::cerr << "studio host label column setup stdout:\n" << label_process.stdout_text << "\n";
        std::cerr << "studio host label column setup stderr:\n" << label_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(label_process.exit_code == 0,
           "#1518: label column setup JSON should exit successfully");
    expect_contains(label_process.stdout_text, "\"isLabel\": true",
                    "#1518: label column setup JSON should preserve label identity");
    expect_contains(label_process.stdout_text, "\"pageSetupAvailable\": false",
                    "#1518: label column setup JSON should not fabricate page setup availability");
    expect_contains(label_process.stdout_text, "\"columnSetupAvailable\": true",
                    "#1518: label column setup JSON should expose column setup availability");
    expect_contains(label_process.stdout_text, "\"columnCount\": 2",
                    "#1518: label column setup JSON should expose column counts");
    expect_contains(label_process.stdout_text, "\"columnWidth\": 3600",
                    "#1518: label column setup JSON should expose column widths");
    expect_contains(label_process.stdout_text, "\"columnSpacing\": 120",
                    "#1518: label column setup JSON should expose column spacing");
    expect_contains(label_process.stdout_text, "\"previewBoundsAvailable\": false",
                    "#2017: label column setup JSON should not fabricate live preview availability");
    expect_contains(label_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2017: label column setup JSON should preserve zero live preview left bounds");
    expect_contains(label_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2017: label column setup JSON should preserve zero live preview top bounds");
    expect_contains(label_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2017: label column setup JSON should preserve zero live preview right bounds");
    expect_contains(label_process.stdout_text, "\"previewBoundsBottom\": 0",
                    "#2017: label column setup JSON should preserve zero live preview bottom bounds");
    expect_contains(label_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2017: label column setup JSON should preserve zero live preview widths");
    expect_contains(label_process.stdout_text, "\"previewBoundsHeight\": 0",
                    "#2017: label column setup JSON should preserve zero live preview heights");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2017: label column setup JSON should not fabricate deleted preview availability");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview left bounds");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview top bounds");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview right bounds");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview bottom bounds");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview widths");
    expect_contains(label_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                    "#2017: label column setup JSON should preserve zero deleted preview heights");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_count_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_count_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& updated_count,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_column_setup_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "COLS",
                "--property-value", updated_count,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-count field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-count field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1835: report/label stable column-count field update should exit successfully");
        const auto column_count_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "COLS"
        });
        expect(column_count_property.ok && column_count_property.exists &&
                   column_count_property.value == updated_count,
               "#1835: report/label stable column-count field update should persist the COLS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1835: report/label stable column-count field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1835: label stable column-count field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2031: stable-selected report/label column-count update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1835: report/label stable column-count field update should not fabricate page setup availability");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1835: report/label stable column-count field update should preserve column setup availability");
        expect_contains(update_process.stdout_text, "\"columnCount\": " + updated_count,
                        "#1835: report/label stable column-count field update should refresh column counts");
        expect_contains(update_process.stdout_text, "\"columnWidth\": 3600",
                        "#1835: report/label stable column-count field update should preserve memo-derived column widths");
        expect_contains(update_process.stdout_text, "\"columnSpacing\": 120",
                        "#1835: report/label stable column-count field update should preserve memo-derived column spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 3",
                        "#1835: report/label stable column-count field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1835: report/label stable column-count field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1835: report/label stable column-count field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_count + "\""
            },
            "#1835: report/label stable column-count field update should refresh selected direct-field provenance");
    };

    run_column_count_update(temp_root / "column_count_stable.frx",
                            "column_count_stable.frx",
                            "4",
                            "report");
    run_column_count_update(temp_root / "column_count_stable.lbx",
                            "column_count_stable.lbx",
                            "5",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_count_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_count_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_column_setup_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "COLS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-count field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-count field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1835: report/label stable column-count field clear should exit successfully");
        const auto column_count_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "COLS"
        });
        expect(column_count_property.ok && column_count_property.exists &&
                   column_count_property.value.empty(),
               "#1835: report/label stable column-count field clear should blank the COLS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1835: report/label stable column-count field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1835: label stable column-count field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2031: stable-selected report/label column-count clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1835: report/label stable column-count field clear should not fabricate page setup availability");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1835: report/label stable column-count field clear should preserve column setup availability");
        expect_contains(clear_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1835: report/label stable column-count field clear should clear column-count availability");
        expect_contains(clear_process.stdout_text, "\"columnCount\": 0",
                        "#1835: report/label stable column-count field clear should clear column counts");
        expect_contains(clear_process.stdout_text, "\"columnWidth\": 3600",
                        "#1835: report/label stable column-count field clear should preserve memo-derived column widths");
        expect_contains(clear_process.stdout_text, "\"columnSpacing\": 120",
                        "#1835: report/label stable column-count field clear should preserve memo-derived column spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 2",
                        "#1835: report/label stable column-count field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1835: report/label stable column-count field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1835: report/label stable column-count field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1835: report/label stable column-count field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1835: report/label stable column-count field clear should remove direct COLS provenance");
    };

    run_column_count_clear(temp_root / "column_count_clear_stable.frx",
                           "column_count_clear_stable.frx",
                           "report");
    run_column_count_clear(temp_root / "column_count_clear_stable.lbx",
                           "column_count_clear_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_count_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_count_update = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& updated_count,
                                                     const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_column_setup_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "COLS",
                "--property-value", updated_count,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-count field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-count field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1835: report/label stable deleted column-count field update should exit successfully");
        const auto column_count_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "COLS"
        });
        expect(column_count_property.ok && column_count_property.exists &&
                   column_count_property.value == updated_count,
               "#1835: report/label stable deleted column-count field update should persist the COLS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1835: report/label stable deleted column-count field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1835: label stable deleted column-count field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2031: stable-selected deleted report/label column-count update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1835: report/label stable deleted column-count field update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#3815: report/label stable deleted column-count field update should expose effective column setup");
        expect_contains(update_process.stdout_text, "\"columnCountAvailable\": true",
                        "#3815: report/label stable deleted column-count field update should expose effective availability");
        expect_contains(update_process.stdout_text, "\"columnCount\": " + updated_count,
                        "#3815: report/label stable deleted column-count field update should expose the effective value");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1835: report/label stable deleted column-count field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1835: report/label stable deleted column-count field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1835: report/label stable deleted column-count field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1835: report/label stable deleted column-count field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_count + "\""
            },
            "#1835: report/label stable deleted column-count field update should refresh deleted setting provenance");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_count + "\""
            },
            "#1835: report/label stable deleted column-count field update should refresh selected deleted settings");
    };

    run_deleted_column_count_update(temp_root / "deleted_column_count_stable.frx",
                                    "deleted_column_count_stable.frx",
                                    "4",
                                    "report");
    run_deleted_column_count_update(temp_root / "deleted_column_count_stable.lbx",
                                    "deleted_column_count_stable.lbx",
                                    "5",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_column_count_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_count_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_count_clear = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_column_setup_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "COLS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-count field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-count field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1835: report/label stable deleted column-count field clear should exit successfully");
        const auto column_count_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "COLS"
        });
        expect(column_count_property.ok && column_count_property.exists &&
                   column_count_property.value.empty(),
               "#1835: report/label stable deleted column-count field clear should blank the COLS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1835: report/label stable deleted column-count field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1835: label stable deleted column-count field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2031: stable-selected deleted report/label column-count clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1835: report/label stable deleted column-count field clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#3815: report/label stable deleted column-count field clear should preserve effective column setup");
        expect_contains(clear_process.stdout_text, "\"columnCountAvailable\": false",
                        "#3815: report/label stable deleted column-count field clear should clear effective availability");
        expect_contains(clear_process.stdout_text, "\"columnCount\": 0",
                        "#3815: report/label stable deleted column-count field clear should reset the effective value");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1835: report/label stable deleted column-count field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 2",
                        "#1835: report/label stable deleted column-count field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1835: report/label stable deleted column-count field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1835: report/label stable deleted column-count field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1835: report/label stable deleted column-count field clear should preserve remaining deleted setting provenance");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1835: report/label stable deleted column-count field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1835: report/label stable deleted column-count field clear should remove direct COLS provenance");
    };

    run_deleted_column_count_clear(temp_root / "deleted_column_count_clear_stable.frx",
                                   "deleted_column_count_clear_stable.frx",
                                   "report");
    run_deleted_column_count_clear(temp_root / "deleted_column_count_clear_stable.lbx",
                                   "deleted_column_count_clear_stable.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#if !defined(COPPERFIN_REPORT_DIRECT_SETTING_FIELDS_SKIP_HOST_SMOKE)
void test_studio_host_json_ignores_invalid_direct_report_column_setup_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_invalid_direct_column_setup_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_invalid_direct_column_setup_layout = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_invalid_direct_column_setup_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " invalid direct column setup summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " invalid direct column setup summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1734: invalid direct column setup fields should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1734: invalid direct column setup layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1734: invalid direct column setup label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1734: invalid direct column setup fields should not fabricate column setup availability");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1734: invalid direct column count should not advertise column-count availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1734: invalid direct column width should not advertise column-width availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1734: invalid direct column spacing should not advertise column-spacing availability");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 0",
                        "#1734: invalid direct column count should keep the default column-count value inert");
        expect_contains(summary_process.stdout_text, "\"columnWidth\": 0",
                        "#1734: invalid direct column width should keep the default column-width value inert");
        expect_contains(summary_process.stdout_text, "\"columnSpacing\": 0",
                        "#1734: invalid direct column spacing should keep the default column-spacing value inert");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 3",
                        "#1734: invalid direct column settings should still be counted as live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1734: invalid direct column settings should still be counted as deleted raw settings");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"many\"",
                        "#1734: invalid direct column-count provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"wide?\"",
                        "#1734: invalid direct column-width provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"spaced?\"",
                        "#1734: invalid direct column-spacing provenance should remain inspectable");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1734: invalid direct live column settings selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1734: invalid direct live column settings should advertise selected-settings availability");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1734: invalid direct live column settings should expose settings selection kind");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"many\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 3",
                "\"value\": \"wide?\"",
                "\"name\": \"COLSPACING\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"spaced?\""
            },
            "#1734: invalid direct live column selection should expose raw selected-settings metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1734: invalid direct deleted column settings selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1734: invalid direct deleted column settings should advertise selected-settings availability");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1734: invalid direct deleted column settings should expose settings selection kind");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"value\": \"deleted-many\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"deleted-wide?\"",
                "\"name\": \"COLSPACING\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 4",
                "\"value\": \"deleted-spaced?\""
            },
            "#1734: invalid direct deleted column selection should expose raw selected-settings metadata");
    };

    run_invalid_direct_column_setup_layout(temp_root / "invalid_direct_column_setup.frx",
                                           "invalid_direct_column_setup.frx",
                                           "report");
    run_invalid_direct_column_setup_layout(temp_root / "invalid_direct_column_setup.lbx",
                                           "invalid_direct_column_setup.lbx",
                                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

}  // namespace cf_test_studio_host_json
