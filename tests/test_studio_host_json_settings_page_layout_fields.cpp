// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_invalid_direct_margin_grid_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "bottom?", "vertical?", "horizontal?", "invalid-direct-live-grid-settings-guid"},
        {"1", "53", "deleted-bottom?", "deleted-vertical?", "deleted-horizontal?",
         "invalid-direct-deleted-grid-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1735: synthetic report table with invalid direct margin/grid fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1735: synthetic report table should mark invalid direct margin/grid settings deleted");
}

void write_synthetic_report_table_for_bottom_margin_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "BOTMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "TOPMARGIN=10\nGRIDV=4\nGRIDH=8", "20", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1541: synthetic report table for bottom margin field JSON should be created");
}

void write_synthetic_report_table_for_deleted_bottom_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_bottom_margin_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1583: synthetic report table should mark bottom-margin settings deleted");
}

void write_synthetic_report_table_for_stable_bottom_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_bottom_margin_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1830: stable bottom-margin fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1830: stable bottom-margin fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_bottom_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_bottom_margin_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1830: stable deleted bottom-margin fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1830: stable deleted bottom-margin fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_left_margin_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "LEFTMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "TOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "15", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3015: synthetic report table for left-margin field JSON should be created");
}

void write_synthetic_report_table_for_deleted_left_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_left_margin_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#3015: synthetic report table should mark left-margin settings deleted");
}

void write_synthetic_report_table_for_stable_left_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_left_margin_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#3015: stable left-margin fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#3015: stable left-margin fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_left_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_left_margin_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#3015: stable deleted left-margin fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#3015: stable deleted left-margin fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_right_margin_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "RIGHTMARGI", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "TOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "25", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3015: synthetic report table for right-margin field JSON should be created");
}

void write_synthetic_report_table_for_deleted_right_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_right_margin_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#3015: synthetic report table should mark right-margin settings deleted");
}

void write_synthetic_report_table_for_stable_right_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_right_margin_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#3015: stable right-margin fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#3015: stable right-margin fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_right_margin_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_right_margin_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#3015: stable deleted right-margin fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#3015: stable deleted right-margin fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_grid_vertical_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "GRIDV", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "TOPMARGIN=10\nBOTMARGIN=20\nGRIDH=8", "4", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1542: synthetic report table for vertical grid field JSON should be created");
}

void write_synthetic_report_table_for_deleted_grid_vertical_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_grid_vertical_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1585: synthetic report table should mark vertical-grid settings deleted");
}

void write_synthetic_report_table_for_stable_grid_vertical_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_grid_vertical_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1831: stable vertical-grid fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1831: stable vertical-grid fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_grid_vertical_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_grid_vertical_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1831: stable deleted vertical-grid fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1831: stable deleted vertical-grid fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_grid_horizontal_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "GRIDH", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "TOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4", "8", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1543: synthetic report table for horizontal grid field JSON should be created");
}

void write_synthetic_report_table_for_deleted_grid_horizontal_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_grid_horizontal_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1587: synthetic report table should mark horizontal-grid settings deleted");
}

void write_synthetic_report_table_for_stable_grid_horizontal_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_grid_horizontal_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1832: stable horizontal-grid fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1832: stable horizontal-grid fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_grid_horizontal_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_grid_horizontal_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1832: stable deleted horizontal-grid fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1832: stable deleted horizontal-grid fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_orientation_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "ORIENTATIO", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "PAPERSIZE=1\nTOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "0", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1544: synthetic report table for orientation field JSON should be created");
}

void write_synthetic_report_table_for_deleted_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_orientation_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1589: synthetic report table should mark orientation settings deleted");
}

void write_synthetic_report_table_for_stable_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_orientation_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1833: stable orientation fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1833: stable orientation fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_orientation_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_orientation_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1833: stable deleted orientation fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1833: stable deleted orientation fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_paper_size_field_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "PAPERSIZE", .type = 'N', .length = 8U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nTOPMARGIN=10\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "1", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1545: synthetic report table for paper-size field JSON should be created");
}

void write_synthetic_report_table_for_deleted_paper_size_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_paper_size_field_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1591: synthetic report table should mark paper-size settings deleted");
}

void write_synthetic_report_table_for_stable_paper_size_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_paper_size_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#1834: stable paper-size fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#1834: stable paper-size fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_paper_size_field_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_paper_size_field_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#1834: stable deleted paper-size fixture should seed a settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#1834: stable deleted paper-size fixture should preserve the deleted settings state");
}

#include "test_studio_host_json_settings_page_layout_margin_fields.inl"
#include "test_studio_host_json_settings_page_layout_grid_horizontal_fields.inl"
void test_studio_host_json_updates_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_orientation_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_orientation_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& updated_orientation,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_orientation_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "ORIENTATION",
                "--property-value", updated_orientation,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable orientation field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable orientation field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1833: report/label stable orientation field update should exit successfully");
        const auto orientation_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "ORIENTATION"
        });
        expect(orientation_property.ok && orientation_property.exists &&
                   orientation_property.value == updated_orientation,
               "#1833: report/label stable orientation field update should persist the ORIENTATION field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1833: report/label stable orientation field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1833: label stable orientation field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2023: stable-selected report/label orientation update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1833: report/label stable orientation field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"orientationCode\": " + updated_orientation,
                        "#1833: report/label stable orientation field update should refresh orientation codes");
        expect_contains(update_process.stdout_text, "\"paperSizeCode\": 1",
                        "#1833: report/label stable orientation field update should preserve paper-size codes");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#1833: report/label stable orientation field update should preserve top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        "#1833: report/label stable orientation field update should preserve bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        "#1833: report/label stable orientation field update should preserve vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1833: report/label stable orientation field update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 6",
                        "#1833: report/label stable orientation field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1833: report/label stable orientation field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1833: report/label stable orientation field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_orientation + "\""
            },
            "#1833: report/label stable orientation field update should refresh selected direct-field provenance");
    };

    run_orientation_update(temp_root / "orientation_stable.frx",
                           "orientation_stable.frx",
                           "1",
                           "report");
    run_orientation_update(temp_root / "orientation_stable.lbx",
                           "orientation_stable.lbx",
                           "2",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_orientation_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_orientation_clear = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_stable_orientation_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "ORIENTATION",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable orientation field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable orientation field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1833: report/label stable orientation field clear should exit successfully");
        const auto orientation_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "ORIENTATION"
        });
        expect(orientation_property.ok && orientation_property.exists && orientation_property.value.empty(),
               "#1833: report/label stable orientation field clear should blank the ORIENTATION field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1833: report/label stable orientation field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1833: label stable orientation field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2023: stable-selected report/label orientation clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1833: report/label stable orientation field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"orientationAvailable\": false",
                        "#1833: report/label stable orientation field clear should clear orientation availability");
        expect_contains(clear_process.stdout_text, "\"orientationCode\": 0",
                        "#1833: report/label stable orientation field clear should clear orientation codes");
        expect_contains(clear_process.stdout_text, "\"paperSizeCode\": 1",
                        "#1833: report/label stable orientation field clear should preserve paper-size codes");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#1833: report/label stable orientation field clear should preserve top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        "#1833: report/label stable orientation field clear should preserve bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        "#1833: report/label stable orientation field clear should preserve vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1833: report/label stable orientation field clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 5",
                        "#1833: report/label stable orientation field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1833: report/label stable orientation field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1833: report/label stable orientation field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            "#1833: report/label stable orientation field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1833: report/label stable orientation field clear should remove direct ORIENTATION provenance");
    };

    run_orientation_clear(temp_root / "orientation_clear_stable.frx",
                          "orientation_clear_stable.frx",
                          "report");
    run_orientation_clear(temp_root / "orientation_clear_stable.lbx",
                          "orientation_clear_stable.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_orientation_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_orientation_update = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& updated_orientation,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_orientation_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "ORIENTATION",
                "--property-value", updated_orientation,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted orientation field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted orientation field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1833: report/label stable deleted orientation field update should exit successfully");
        const auto orientation_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "ORIENTATION"
        });
        expect(orientation_property.ok && orientation_property.exists &&
                   orientation_property.value == updated_orientation,
               "#1833: report/label stable deleted orientation field update should persist the ORIENTATION field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1833: report/label stable deleted orientation field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1833: label stable deleted orientation field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2023: stable-selected deleted report/label orientation update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted orientation field update should expose effective page setup");
        expect_contains(update_process.stdout_text, "\"orientationAvailable\": true",
                        "#3815: report/label stable deleted orientation field update should expose effective availability");
        expect_contains(update_process.stdout_text, "\"orientationCode\": " + updated_orientation,
                        "#3815: report/label stable deleted orientation field update should expose the effective value");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1833: report/label stable deleted orientation field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1833: report/label stable deleted orientation field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1833: report/label stable deleted orientation field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1833: report/label stable deleted orientation field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_orientation + "\""
            },
            "#1833: report/label stable deleted orientation field update should refresh selected deleted settings");
    };

    run_deleted_orientation_update(temp_root / "deleted_orientation_stable.frx",
                                   "deleted_orientation_stable.frx",
                                   "1",
                                   "report");
    run_deleted_orientation_update(temp_root / "deleted_orientation_stable.lbx",
                                   "deleted_orientation_stable.lbx",
                                   "2",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_orientation_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_orientation_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_orientation_clear = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_orientation_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "ORIENTATION",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted orientation field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted orientation field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1833: report/label stable deleted orientation field clear should exit successfully");
        const auto orientation_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "ORIENTATION"
        });
        expect(orientation_property.ok && orientation_property.exists && orientation_property.value.empty(),
               "#1833: report/label stable deleted orientation field clear should blank the ORIENTATION field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1833: report/label stable deleted orientation field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1833: label stable deleted orientation field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2023: stable-selected deleted report/label orientation clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted orientation field clear should preserve effective page setup");
        expect_contains(clear_process.stdout_text, "\"orientationAvailable\": false",
                        "#3815: report/label stable deleted orientation field clear should clear effective availability");
        expect_contains(clear_process.stdout_text, "\"orientationCode\": 0",
                        "#3815: report/label stable deleted orientation field clear should reset the effective value");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1833: report/label stable deleted orientation field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 5",
                        "#1833: report/label stable deleted orientation field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1833: report/label stable deleted orientation field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1833: report/label stable deleted orientation field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            "#1833: report/label stable deleted orientation field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1833: report/label stable deleted orientation field clear should remove direct ORIENTATION provenance");
    };

    run_deleted_orientation_clear(temp_root / "deleted_orientation_clear_stable.frx",
                                  "deleted_orientation_clear_stable.frx",
                                  "report");
    run_deleted_orientation_clear(temp_root / "deleted_orientation_clear_stable.lbx",
                                  "deleted_orientation_clear_stable.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_paper_size_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_paper_size_update = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& updated_paper_size,
                                           const std::string& label) {
        write_synthetic_report_table_for_stable_paper_size_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "PAPERSIZE",
                "--property-value", updated_paper_size,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable paper-size field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable paper-size field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1834: report/label stable paper-size field update should exit successfully");
        const auto paper_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "PAPERSIZE"
        });
        expect(paper_size_property.ok && paper_size_property.exists &&
                   paper_size_property.value == updated_paper_size,
               "#1834: report/label stable paper-size field update should persist the PAPERSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1834: report/label stable paper-size field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1834: label stable paper-size field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2021: stable-selected report/label paper-size update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1834: report/label stable paper-size field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"orientationCode\": 0",
                        "#1834: report/label stable paper-size field update should preserve orientation codes");
        expect_contains(update_process.stdout_text, "\"paperSizeCode\": " + updated_paper_size,
                        "#1834: report/label stable paper-size field update should refresh paper-size codes");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#1834: report/label stable paper-size field update should preserve top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        "#1834: report/label stable paper-size field update should preserve bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        "#1834: report/label stable paper-size field update should preserve vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1834: report/label stable paper-size field update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 6",
                        "#1834: report/label stable paper-size field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1834: report/label stable paper-size field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1834: report/label stable paper-size field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_paper_size + "\""
            },
            "#1834: report/label stable paper-size field update should refresh selected direct-field provenance");
    };

    run_paper_size_update(temp_root / "paper_size_stable.frx",
                          "paper_size_stable.frx",
                          "9",
                          "report");
    run_paper_size_update(temp_root / "paper_size_stable.lbx",
                          "paper_size_stable.lbx",
                          "5",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_paper_size_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_paper_size_clear = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_stable_paper_size_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "PAPERSIZE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable paper-size field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable paper-size field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1834: report/label stable paper-size field clear should exit successfully");
        const auto paper_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "PAPERSIZE"
        });
        expect(paper_size_property.ok && paper_size_property.exists && paper_size_property.value.empty(),
               "#1834: report/label stable paper-size field clear should blank the PAPERSIZE field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1834: report/label stable paper-size field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1834: label stable paper-size field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2021: stable-selected report/label paper-size clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1834: report/label stable paper-size field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"orientationCode\": 0",
                        "#1834: report/label stable paper-size field clear should preserve orientation codes");
        expect_contains(clear_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1834: report/label stable paper-size field clear should clear paper-size availability");
        expect_contains(clear_process.stdout_text, "\"paperSizeCode\": 0",
                        "#1834: report/label stable paper-size field clear should clear paper-size codes");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#1834: report/label stable paper-size field clear should preserve top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        "#1834: report/label stable paper-size field clear should preserve bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        "#1834: report/label stable paper-size field clear should preserve vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1834: report/label stable paper-size field clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 5",
                        "#1834: report/label stable paper-size field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1834: report/label stable paper-size field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1834: report/label stable paper-size field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            "#1834: report/label stable paper-size field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1834: report/label stable paper-size field clear should remove direct PAPERSIZE provenance");
    };

    run_paper_size_clear(temp_root / "paper_size_clear_stable.frx",
                         "paper_size_clear_stable.frx",
                         "report");
    run_paper_size_clear(temp_root / "paper_size_clear_stable.lbx",
                         "paper_size_clear_stable.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_paper_size_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_paper_size_update = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& updated_paper_size,
                                                   const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_paper_size_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "PAPERSIZE",
                "--property-value", updated_paper_size,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted paper-size field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted paper-size field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1834: report/label stable deleted paper-size field update should exit successfully");
        const auto paper_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "PAPERSIZE"
        });
        expect(paper_size_property.ok && paper_size_property.exists &&
                   paper_size_property.value == updated_paper_size,
               "#1834: report/label stable deleted paper-size field update should persist the PAPERSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1834: report/label stable deleted paper-size field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1834: label stable deleted paper-size field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2021: stable-selected deleted report/label paper-size update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted paper-size field update should expose effective page setup");
        expect_contains(update_process.stdout_text, "\"paperSizeAvailable\": true",
                        "#3815: report/label stable deleted paper-size field update should expose effective availability");
        expect_contains(update_process.stdout_text, "\"paperSizeCode\": " + updated_paper_size,
                        "#3815: report/label stable deleted paper-size field update should expose the effective value");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1834: report/label stable deleted paper-size field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1834: report/label stable deleted paper-size field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1834: report/label stable deleted paper-size field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1834: report/label stable deleted paper-size field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_paper_size + "\""
            },
            "#1834: report/label stable deleted paper-size field update should refresh selected deleted settings");
    };

    run_deleted_paper_size_update(temp_root / "deleted_paper_size_stable.frx",
                                  "deleted_paper_size_stable.frx",
                                  "9",
                                  "report");
    run_deleted_paper_size_update(temp_root / "deleted_paper_size_stable.lbx",
                                  "deleted_paper_size_stable.lbx",
                                  "5",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_paper_size_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_paper_size_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_paper_size_clear = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_paper_size_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "PAPERSIZE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted paper-size field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted paper-size field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1834: report/label stable deleted paper-size field clear should exit successfully");
        const auto paper_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "PAPERSIZE"
        });
        expect(paper_size_property.ok && paper_size_property.exists && paper_size_property.value.empty(),
               "#1834: report/label stable deleted paper-size field clear should blank the PAPERSIZE field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1834: report/label stable deleted paper-size field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1834: label stable deleted paper-size field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2021: stable-selected deleted report/label paper-size clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted paper-size field clear should preserve effective page setup");
        expect_contains(clear_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#3815: report/label stable deleted paper-size field clear should clear effective availability");
        expect_contains(clear_process.stdout_text, "\"paperSizeCode\": 0",
                        "#3815: report/label stable deleted paper-size field clear should reset the effective value");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1834: report/label stable deleted paper-size field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 5",
                        "#1834: report/label stable deleted paper-size field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1834: report/label stable deleted paper-size field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1834: report/label stable deleted paper-size field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
            },
            "#1834: report/label stable deleted paper-size field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1834: report/label stable deleted paper-size field clear should remove direct PAPERSIZE provenance");
    };

    run_deleted_paper_size_clear(temp_root / "deleted_paper_size_clear_stable.frx",
                                 "deleted_paper_size_clear_stable.frx",
                                 "report");
    run_deleted_paper_size_clear(temp_root / "deleted_paper_size_clear_stable.lbx",
                                 "deleted_paper_size_clear_stable.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#if !defined(COPPERFIN_REPORT_DIRECT_SETTING_FIELDS_SKIP_HOST_SMOKE)
void test_studio_host_json_ignores_invalid_direct_report_margin_grid_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_invalid_direct_margin_grid_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_invalid_direct_margin_grid_layout = [&](const fs::path& asset_path,
                                                           const std::string& title,
                                                           const std::string& label) {
        write_synthetic_report_table_for_invalid_direct_margin_grid_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " invalid direct margin/grid summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " invalid direct margin/grid summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1735: invalid direct margin/grid fields should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1735: invalid direct margin/grid layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1735: invalid direct margin/grid label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1735: invalid direct margin/grid fields should not fabricate page setup availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1735: invalid direct bottom margin should not advertise bottom-margin availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1735: invalid direct vertical grid should not advertise vertical-grid availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1735: invalid direct horizontal grid should not advertise horizontal-grid availability");
        expect_contains(summary_process.stdout_text, "\"bottomMargin\": 0",
                        "#1735: invalid direct bottom margin should keep the default bottom-margin value inert");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 0",
                        "#1735: invalid direct vertical grid should keep the default vertical-grid value inert");
        expect_contains(summary_process.stdout_text, "\"gridHorizontal\": 0",
                        "#1735: invalid direct horizontal grid should keep the default horizontal-grid value inert");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 3",
                        "#1735: invalid direct margin/grid settings should still be counted as live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1735: invalid direct margin/grid settings should still be counted as deleted raw settings");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"bottom?\"",
                        "#1735: invalid direct bottom-margin provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"vertical?\"",
                        "#1735: invalid direct vertical-grid provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"horizontal?\"",
                        "#1735: invalid direct horizontal-grid provenance should remain inspectable");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1735: invalid direct live margin/grid settings selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1735: invalid direct live margin/grid settings should advertise selected-settings availability");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1735: invalid direct live margin/grid settings should expose settings selection kind");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"BOTMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"bottom?\"",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 3",
                "\"value\": \"vertical?\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"horizontal?\""
            },
            "#1735: invalid direct live margin/grid selection should expose raw selected-settings metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1735: invalid direct deleted margin/grid settings selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1735: invalid direct deleted margin/grid settings should advertise selected-settings availability");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1735: invalid direct deleted margin/grid settings should expose settings selection kind");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"BOTMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"value\": \"deleted-bottom?\"",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"deleted-vertical?\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 4",
                "\"value\": \"deleted-horizontal?\""
            },
            "#1735: invalid direct deleted margin/grid selection should expose raw selected-settings metadata");
    };

    run_invalid_direct_margin_grid_layout(temp_root / "invalid_direct_margin_grid.frx",
                                          "invalid_direct_margin_grid.frx",
                                          "report");
    run_invalid_direct_margin_grid_layout(temp_root / "invalid_direct_margin_grid.lbx",
                                          "invalid_direct_margin_grid.lbx",
                                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

}  // namespace cf_test_studio_host_json
