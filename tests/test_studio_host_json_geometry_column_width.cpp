// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_column_width_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "COLWIDTH", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "COLS=2\nCOLSPACING=120", "3600", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1539: synthetic report table for column width field JSON should be created");
}

void write_synthetic_report_table_for_deleted_column_width_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_width_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1595: synthetic report table should mark column-width settings deleted");
}

void write_synthetic_report_table_for_stable_column_width_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_column_width_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1836: stable column-width fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1836: stable column-width fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_column_width_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_column_width_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1836: stable deleted column-width fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1836: stable deleted column-width fixture should preserve the deleted settings state");
}

void test_studio_host_json_updates_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_width_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_width_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& updated_width,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_column_width_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "COLWIDTH",
                "--property-value", updated_width,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-width field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-width field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1836: report/label stable column-width field update should exit successfully");
        const auto column_width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "COLWIDTH"
        });
        expect(column_width_property.ok && column_width_property.exists &&
                   column_width_property.value == updated_width,
               "#1836: report/label stable column-width field update should persist the COLWIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1836: report/label stable column-width field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1836: label stable column-width field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2033: stable-selected report/label column-width update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1836: report/label stable column-width field update should not fabricate page setup availability");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1836: report/label stable column-width field update should preserve column setup availability");
        expect_contains(update_process.stdout_text, "\"columnCount\": 2",
                        "#1836: report/label stable column-width field update should preserve memo-derived column counts");
        expect_contains(update_process.stdout_text, "\"columnWidth\": " + updated_width,
                        "#1836: report/label stable column-width field update should refresh column widths");
        expect_contains(update_process.stdout_text, "\"columnSpacing\": 120",
                        "#1836: report/label stable column-width field update should preserve memo-derived column spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 3",
                        "#1836: report/label stable column-width field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1836: report/label stable column-width field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1836: report/label stable column-width field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_width + "\""
            },
            "#1836: report/label stable column-width field update should refresh selected direct-field provenance");
    };

    run_column_width_update(temp_root / "column_width_stable.frx",
                            "column_width_stable.frx",
                            "2800",
                            "report");
    run_column_width_update(temp_root / "column_width_stable.lbx",
                            "column_width_stable.lbx",
                            "3000",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_width_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_width_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_column_width_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "COLWIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-width field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-width field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1836: report/label stable column-width field clear should exit successfully");
        const auto column_width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "COLWIDTH"
        });
        expect(column_width_property.ok && column_width_property.exists &&
                   column_width_property.value.empty(),
               "#1836: report/label stable column-width field clear should blank the COLWIDTH field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1836: report/label stable column-width field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1836: label stable column-width field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2033: stable-selected report/label column-width clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1836: report/label stable column-width field clear should not fabricate page setup availability");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1836: report/label stable column-width field clear should preserve column setup availability");
        expect_contains(clear_process.stdout_text, "\"columnCount\": 2",
                        "#1836: report/label stable column-width field clear should preserve memo-derived column counts");
        expect_contains(clear_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1836: report/label stable column-width field clear should clear column-width availability");
        expect_contains(clear_process.stdout_text, "\"columnWidth\": 0",
                        "#1836: report/label stable column-width field clear should clear column widths");
        expect_contains(clear_process.stdout_text, "\"columnSpacing\": 120",
                        "#1836: report/label stable column-width field clear should preserve memo-derived column spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 2",
                        "#1836: report/label stable column-width field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1836: report/label stable column-width field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1836: report/label stable column-width field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1836: report/label stable column-width field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1836: report/label stable column-width field clear should remove direct COLWIDTH provenance");
    };

    run_column_width_clear(temp_root / "column_width_clear_stable.frx",
                           "column_width_clear_stable.frx",
                           "report");
    run_column_width_clear(temp_root / "column_width_clear_stable.lbx",
                           "column_width_clear_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_width_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_width_update = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& updated_width,
                                                     const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_column_width_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "COLWIDTH",
                "--property-value", updated_width,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-width field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-width field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1836: report/label stable deleted column-width field update should exit successfully");
        const auto column_width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "COLWIDTH"
        });
        expect(column_width_property.ok && column_width_property.exists &&
                   column_width_property.value == updated_width,
               "#1836: report/label stable deleted column-width field update should persist the COLWIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1836: report/label stable deleted column-width field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1836: label stable deleted column-width field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2033: stable-selected deleted report/label column-width update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1836: report/label stable deleted column-width field update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#3815: report/label stable deleted column-width field update should expose effective column setup");
        expect_contains(update_process.stdout_text, "\"columnWidthAvailable\": true",
                        "#3815: report/label stable deleted column-width field update should expose effective availability");
        expect_contains(update_process.stdout_text, "\"columnWidth\": " + updated_width,
                        "#3815: report/label stable deleted column-width field update should expose the effective value");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1836: report/label stable deleted column-width field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1836: report/label stable deleted column-width field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1836: report/label stable deleted column-width field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1836: report/label stable deleted column-width field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_width + "\""
            },
            "#1836: report/label stable deleted column-width field update should refresh deleted setting provenance");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_width + "\""
            },
            "#1836: report/label stable deleted column-width field update should refresh selected deleted settings");
    };

    run_deleted_column_width_update(temp_root / "deleted_column_width_stable.frx",
                                    "deleted_column_width_stable.frx",
                                    "4800",
                                    "report");
    run_deleted_column_width_update(temp_root / "deleted_column_width_stable.lbx",
                                    "deleted_column_width_stable.lbx",
                                    "2400",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_width_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_width_clear = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_column_width_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "COLWIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-width field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-width field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1836: report/label stable deleted column-width field clear should exit successfully");
        const auto column_width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "COLWIDTH"
        });
        expect(column_width_property.ok && column_width_property.exists &&
                   column_width_property.value.empty(),
               "#1836: report/label stable deleted column-width field clear should blank the COLWIDTH field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1836: report/label stable deleted column-width field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1836: label stable deleted column-width field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2033: stable-selected deleted report/label column-width clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1836: report/label stable deleted column-width field clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#3815: report/label stable deleted column-width field clear should preserve effective column setup");
        expect_contains(clear_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#3815: report/label stable deleted column-width field clear should clear effective availability");
        expect_contains(clear_process.stdout_text, "\"columnWidth\": 0",
                        "#3815: report/label stable deleted column-width field clear should reset the effective value");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1836: report/label stable deleted column-width field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 2",
                        "#1836: report/label stable deleted column-width field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1836: report/label stable deleted column-width field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1836: report/label stable deleted column-width field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1836: report/label stable deleted column-width field clear should preserve remaining deleted setting provenance");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1836: report/label stable deleted column-width field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1836: report/label stable deleted column-width field clear should remove direct COLWIDTH provenance");
    };

    run_deleted_column_width_clear(temp_root / "deleted_column_width_clear_stable.frx",
                                   "deleted_column_width_clear_stable.frx",
                                   "report");
    run_deleted_column_width_clear(temp_root / "deleted_column_width_clear_stable.lbx",
                                   "deleted_column_width_clear_stable.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
