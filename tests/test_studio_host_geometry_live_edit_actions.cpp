// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
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
    expect(create_result.ok, "#1452: synthetic FRX table for report layout JSON should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#1452: synthetic FRX table should mark deleted layout objects");
}

void write_synthetic_report_table_for_layout_distribution_json(const std::filesystem::path& report_path) {
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
        {"8", "0", "middle.value", "175", "2600", "50", "200", "middle-field-guid"},
        {"8", "0", "right.value", "700", "2600", "50", "200", "right-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1469: synthetic FRX table for report layout distribution should be created");
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
    expect(create_result.ok, "#1470: synthetic FRX table for report layout reorder should be created");
}

#include "test_studio_host_geometry_live_edit_actions_nudge.inl"
#include "test_studio_host_geometry_live_edit_actions_align.inl"

void test_studio_host_json_resizes_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_resize_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_resize = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " resize live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " resize live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1626: live report/label layout object resize geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1626: live report/label layout object resize geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto resize_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "field-guid",
                "--resize-object",
                "--resize-mode", "size",
                "--anchor-unique-id", "label-guid",
                "--resize-target-unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (resize_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout resize stdout:\n"
                      << resize_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout resize stderr:\n"
                      << resize_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(resize_process.exit_code == 0,
               "#1626: live edited report/label layout object resize should exit successfully");
        expect(visual_object_property(asset_path, "field-guid", "HPOS") == "300" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "1800" &&
                   visual_object_property(asset_path, "field-guid", "HEIGHT") == "350",
               "#1626: live edited report/label layout object resize should apply anchor size and preserve edited left position");
        expect(visual_object_property(asset_path, "label-guid", "WIDTH") == "1800" &&
                   visual_object_property(asset_path, "label-guid", "HEIGHT") == "350",
               "#1626: live edited report/label layout object resize should preserve anchor size");
        expect_contains(resize_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1626: live edited report/label layout object resize should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(resize_process.stdout_text, "\"isLabel\": true",
                            "#1626: live edited label layout object resize should retain label identity");
        }
        expect_contains(resize_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1885: live edited report/label layout object resize should preserve preview availability");
        expect_contains(resize_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1885: live edited report/label layout object resize should preserve preview left bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1885: live edited report/label layout object resize should preserve preview top bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1885: live edited report/label layout object resize should preserve preview right bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1885: live edited report/label layout object resize should preserve preview bottom bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1885: live edited report/label layout object resize should preserve preview widths");
        expect_contains(resize_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1885: live edited report/label layout object resize should preserve preview heights");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1885: live edited report/label layout object resize should preserve deleted preview availability");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1885: live edited report/label layout object resize should preserve deleted preview left bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1885: live edited report/label layout object resize should preserve deleted preview top bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1885: live edited report/label layout object resize should preserve deleted preview right bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1885: live edited report/label layout object resize should preserve deleted preview bottom bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1885: live edited report/label layout object resize should preserve deleted preview widths");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1885: live edited report/label layout object resize should preserve deleted preview heights");
        expect_contains(resize_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1626: live edited report/label layout object resize should preserve selected-object availability");
        expect_contains(resize_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1626: live edited report/label layout object resize should preserve containing-section availability");
        expect_contains(resize_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1626: live edited report/label layout object resize should serialize containing-section metadata");
        expect_contains_in_order(
            resize_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 950",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"left\": 300",
                "\"top\": 2600",
                "\"width\": 1800",
                "\"right\": 2100",
                "\"height\": 350",
                "\"bottom\": 2950"
            },
            "#1626: live edited report/label layout object resize should refresh selected resized geometry and section metadata");
    };

    run_live_edited_resize(temp_root / "resize_live_edited.frx",
                           "resize_live_edited.frx",
                           "report");
    run_live_edited_resize(temp_root / "resize_live_edited.lbx",
                           "resize_live_edited.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_snaps_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_snap_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_snap = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " snap live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " snap live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1627: live report/label layout object snap geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1627: live report/label layout object snap geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "375");
        set_live_geometry("VPOS", "2550");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto snap_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "field-guid",
                "--snap-object",
                "--snap-mode", "both",
                "--grid-width", "700",
                "--grid-height", "750",
                "--snap-target-unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (snap_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout snap stdout:\n"
                      << snap_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout snap stderr:\n"
                      << snap_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(snap_process.exit_code == 0,
               "#1627: live edited report/label layout object snap should exit successfully");
        expect(visual_object_property(asset_path, "field-guid", "HPOS") == "700" &&
                   visual_object_property(asset_path, "field-guid", "VPOS") == "2250" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "field-guid", "HEIGHT") == "500",
               "#1627: live edited report/label layout object snap should apply grid position and preserve edited size fields");
        expect_contains(snap_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1627: live edited report/label layout object snap should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(snap_process.stdout_text, "\"isLabel\": true",
                            "#1627: live edited label layout object snap should retain label identity");
        }
        expect_contains(snap_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1886: live edited report/label layout object snap should preserve preview availability");
        expect_contains(snap_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1886: live edited report/label layout object snap should preserve preview left bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1886: live edited report/label layout object snap should preserve preview top bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1886: live edited report/label layout object snap should preserve preview right bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1886: live edited report/label layout object snap should preserve preview bottom bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1886: live edited report/label layout object snap should preserve preview widths");
        expect_contains(snap_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1886: live edited report/label layout object snap should preserve preview heights");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1886: live edited report/label layout object snap should preserve deleted preview availability");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1886: live edited report/label layout object snap should preserve deleted preview left bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1886: live edited report/label layout object snap should preserve deleted preview top bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1886: live edited report/label layout object snap should preserve deleted preview right bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1886: live edited report/label layout object snap should preserve deleted preview bottom bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1886: live edited report/label layout object snap should preserve deleted preview widths");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1886: live edited report/label layout object snap should preserve deleted preview heights");
        expect_contains(snap_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1627: live edited report/label layout object snap should preserve selected-object availability");
        expect_contains(snap_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1627: live edited report/label layout object snap should preserve containing-section availability");
        expect_contains(snap_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1627: live edited report/label layout object snap should serialize containing-section metadata");
        expect_contains_in_order(
            snap_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 250",
                "\"sectionRelativeBottom\": 750",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"left\": 700",
                "\"top\": 2250",
                "\"width\": 250",
                "\"right\": 950",
                "\"height\": 500",
                "\"bottom\": 2750"
            },
            "#1627: live edited report/label layout object snap should refresh selected snapped geometry and section metadata");
    };

    run_live_edited_snap(temp_root / "snap_live_edited.frx",
                         "snap_live_edited.frx",
                         "report");
    run_live_edited_snap(temp_root / "snap_live_edited.lbx",
                         "snap_live_edited.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_live_edited_then_deleted_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_restore_live_edited_deleted_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_delete_restore = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1617: live edited then deleted report/label layout object restore fixture should start live");

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "3",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " live round-trip " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live round-trip " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1617: live report/label layout object round-trip geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1617: live report/label layout object round-trip geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "1400");
        set_live_geometry("WIDTH", "2400");
        set_live_geometry("HEIGHT", "900");

        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--delete-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited delete before restore stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited delete before restore stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1617: live edited report/label layout object delete before restore should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1617: live edited report/label layout object delete before restore should mark the DBF record deleted");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--restore-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited delete restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited delete restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1617: live edited then deleted report/label layout object restore should exit successfully");
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1617: live edited then deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "WIDTH"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "1400" &&
                   width_property.ok && width_property.exists && width_property.value == "2400" &&
                   height_property.ok && height_property.exists && height_property.value == "900",
               "#1617: live edited then deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1617: live edited then deleted report/label layout object restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1888: edited label layout object restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1888: edited report/label layout object restore should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1888: edited report/label layout object restore should refresh live preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1888: edited report/label layout object restore should refresh live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 3800",
                        "#1888: edited report/label layout object restore should refresh live preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1888: edited report/label layout object restore should refresh live preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 3800",
                        "#1888: edited report/label layout object restore should refresh live preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1888: edited report/label layout object restore should refresh live preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1888: edited report/label layout object restore should preserve deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1888: edited report/label layout object restore should preserve deleted preview left bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1888: edited report/label layout object restore should preserve deleted preview top bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1888: edited report/label layout object restore should preserve deleted preview right bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1888: edited report/label layout object restore should preserve deleted preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1888: edited report/label layout object restore should preserve deleted preview widths");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1888: edited report/label layout object restore should preserve deleted preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1617: live edited then deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1617: live edited then deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1617: live edited then deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1617: live edited then deleted report/label layout object restore should rehydrate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1617: live edited then deleted report/label layout object restore should serialize containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1500",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"left\": 1400",
                "\"top\": 2600",
                "\"width\": 2400",
                "\"right\": 3800",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1617: live edited then deleted report/label layout object restore should refresh selected live geometry and section metadata");
    };

    run_live_edited_delete_restore(temp_root / "restore_live_edited_deleted.frx",
                                   "restore_live_edited_deleted.frx",
                                   "report");
    run_live_edited_delete_restore(temp_root / "restore_live_edited_deleted.lbx",
                                   "restore_live_edited_deleted.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_live_edited_unplaced_then_deleted_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_restore_live_edited_unplaced_deleted_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_unplaced_delete_restore = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1618: live edited unplaced then deleted report/label layout object restore fixture should start live");

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "3",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " live unplaced round-trip "
                          << property_name << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live unplaced round-trip "
                          << property_name << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1618: live unplaced report/label layout object round-trip geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1618: live unplaced report/label layout object round-trip geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--delete-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited unplaced delete before restore stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited unplaced delete before restore stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1618: live edited unplaced report/label layout object delete before restore should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1618: live edited unplaced report/label layout object delete before restore should mark the DBF record deleted");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--restore-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited unplaced delete restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited unplaced delete restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1618: live edited unplaced then deleted report/label layout object restore should exit successfully");
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1618: live edited unplaced then deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "-300" &&
                   top_property.ok && top_property.exists && top_property.value == "9000" &&
                   height_property.ok && height_property.exists && height_property.value == "700",
               "#1618: live edited unplaced then deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1618: live edited unplaced then deleted report/label layout object restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1890: edited unplaced label layout object restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1890: edited unplaced report/label layout object restore should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1890: edited unplaced report/label layout object restore should preserve live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 3700",
                        "#1890: edited unplaced report/label layout object restore should expand preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 4000",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 9700",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview left bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview top bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview right bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview widths");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1618: live edited unplaced then deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1618: live edited unplaced then deleted report/label layout object restore should not count out-of-band restored objects as placed");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1618: live edited unplaced then deleted report/label layout object restore should add restored objects to unplaced counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1618: live edited unplaced then deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1618: live edited unplaced then deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1618: live edited unplaced then deleted report/label layout object restore should not fabricate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1618: live edited unplaced then deleted report/label layout object restore should serialize null containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 4000",
                "\"right\": 3700",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1618: live edited unplaced then deleted report/label layout object restore should refresh selected unplaced geometry without section metadata");
    };

    run_live_edited_unplaced_delete_restore(temp_root / "restore_live_edited_unplaced_deleted.frx",
                                            "restore_live_edited_unplaced_deleted.frx",
                                            "report");
    run_live_edited_unplaced_delete_restore(temp_root / "restore_live_edited_unplaced_deleted.lbx",
                                            "restore_live_edited_unplaced_deleted.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_distribute_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_distribution = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_distribution_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " distribute live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " distribute live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1629: live report/label layout object distribution geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1629: live report/label layout object distribution geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto distribute_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--distribute-object",
                "--distribution-mode", "horizontal",
                "--distribute-target-unique-id", "left-field-guid",
                "--distribute-target-unique-id", "middle-field-guid",
                "--distribute-target-unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        if (distribute_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout distribution stdout:\n"
                      << distribute_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout distribution stderr:\n"
                      << distribute_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(distribute_process.exit_code == 0,
               "#1629: live edited report/label layout object distribution should exit successfully");
        expect(visual_object_property(asset_path, "left-field-guid", "HPOS") == "100" &&
                   visual_object_property(asset_path, "middle-field-guid", "HPOS") == "400" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "700",
               "#1629: live edited report/label layout object distribution should distribute the edited object between endpoints");
        expect(visual_object_property(asset_path, "middle-field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "middle-field-guid", "HEIGHT") == "500",
               "#1629: live edited report/label layout object distribution should preserve edited size fields");
        expect_contains(distribute_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1629: live edited report/label layout object distribution should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(distribute_process.stdout_text, "\"isLabel\": true",
                            "#1629: live edited label layout object distribution should retain label identity");
        }
        expect_contains(distribute_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1629: live edited report/label layout object distribution should preserve selected-object availability");
        expect_contains(distribute_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1629: live edited report/label layout object distribution should preserve containing-section availability");
        expect_contains(distribute_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1629: live edited report/label layout object distribution should serialize containing-section metadata");
        expect_contains_in_order(
            distribute_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1100",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": 400",
                "\"top\": 2600",
                "\"width\": 250",
                "\"right\": 650",
                "\"height\": 500",
                "\"bottom\": 3100"
            },
            "#1629: live edited report/label layout object distribution should refresh selected distributed geometry and section metadata");
    };

    run_live_edited_distribution(temp_root / "distribute_live_edited.frx",
                                 "distribute_live_edited.frx",
                                 "report");
    run_live_edited_distribution(temp_root / "distribute_live_edited.lbx",
                                 "distribute_live_edited.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_live_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_reorder_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_reorder = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " reorder live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " reorder live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1623: live report/label layout object reorder geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1623: live report/label layout object reorder geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto reorder_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--reorder-object",
                "--unique-id", "middle-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout reorder stdout:\n"
                      << reorder_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout reorder stderr:\n"
                      << reorder_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_process.exit_code == 0,
               "#1623: live edited report/label layout object reorder should exit successfully");
        expect(visual_object_order(asset_path) == "middle-field-guid,left-field-guid,right-field-guid",
               "#1623: live edited report/label layout object reorder should move the edited object before the target");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS") == "300" &&
                   visual_object_property(asset_path, "middle-field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "middle-field-guid", "HEIGHT") == "500",
               "#1623: live edited report/label layout object reorder should preserve edited geometry fields");
        expect_contains(reorder_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1623: live edited report/label layout object reorder should return refreshed report-layout JSON");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1623: live edited report/label layout object reorder should preserve selected-object availability");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1623: live edited report/label layout object reorder should preserve containing-section availability");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1623: live edited report/label layout object reorder should serialize containing-section metadata");
        expect_contains_in_order(
            reorder_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1100",
                "\"sectionObjectIndex\": 2",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": 300",
                "\"top\": 2600",
                "\"width\": 250",
                "\"right\": 550",
                "\"height\": 500",
                "\"bottom\": 3100"
            },
            "#1623: live edited report/label layout object reorder should refresh selected reordered geometry and section metadata");
    };

    run_live_edited_reorder(temp_root / "reorder_live_edited.frx",
                            "reorder_live_edited.frx",
                            "report");
    run_live_edited_reorder(temp_root / "reorder_live_edited.lbx",
                            "reorder_live_edited.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_live_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_reorder_live_edited_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_unplaced_reorder = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " reorder unplaced live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " reorder unplaced live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1624: live unplaced report/label layout object reorder geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1624: live unplaced report/label layout object reorder geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto reorder_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--reorder-object",
                "--unique-id", "middle-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited unplaced layout reorder stdout:\n"
                      << reorder_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited unplaced layout reorder stderr:\n"
                      << reorder_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_process.exit_code == 0,
               "#1624: live edited unplaced report/label layout object reorder should exit successfully");
        expect(visual_object_order(asset_path) == "middle-field-guid,left-field-guid,right-field-guid",
               "#1624: live edited unplaced report/label layout object reorder should move the edited object before the target");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS") == "-300" &&
                   visual_object_property(asset_path, "middle-field-guid", "VPOS") == "9000" &&
                   visual_object_property(asset_path, "middle-field-guid", "HEIGHT") == "700",
               "#1624: live edited unplaced report/label layout object reorder should preserve edited geometry fields");
        expect_contains(reorder_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1624: live edited unplaced report/label layout object reorder should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reorder_process.stdout_text, "\"isLabel\": true",
                            "#1880: live edited unplaced label layout object reorder should retain label identity");
        }
        expect_contains(reorder_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1880: live edited unplaced report/label layout object reorder should keep preview bounds available");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1880: live edited unplaced report/label layout object reorder should expand preview left bounds");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#1880: live edited unplaced report/label layout object reorder should preserve section-origin preview top bounds");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1880: live edited unplaced report/label layout object reorder should preserve preview right bounds");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1880: live edited unplaced report/label layout object reorder should expand preview bottom bounds");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsWidth\": 450",
                        "#1880: live edited unplaced report/label layout object reorder should refresh preview widths");
        expect_contains(reorder_process.stdout_text, "\"previewBoundsHeight\": 7700",
                        "#1880: live edited unplaced report/label layout object reorder should refresh preview heights");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1624: live edited unplaced report/label layout object reorder should preserve selected-object availability");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1624: live edited unplaced report/label layout object reorder should keep containing-section unavailable");
        expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1624: live edited unplaced report/label layout object reorder should serialize null containing section");
        expect_contains_in_order(
            reorder_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 50",
                "\"right\": -250",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1624: live edited unplaced report/label layout object reorder should refresh selected reordered geometry without fabricated section metadata");
    };

    run_live_edited_unplaced_reorder(temp_root / "reorder_live_edited_unplaced.frx",
                                     "reorder_live_edited_unplaced.frx",
                                     "report");
    run_live_edited_unplaced_reorder(temp_root / "reorder_live_edited_unplaced.lbx",
                                     "reorder_live_edited_unplaced.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_live_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_duplicate_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_duplicate = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " duplicate live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " duplicate live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1619: live report/label layout object duplicate geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1619: live report/label layout object duplicate geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto duplicate_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--duplicate-object",
                "--unique-id", "middle-field-guid",
                "--new-unique-id", "middle-edited-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout duplicate stdout:\n"
                      << duplicate_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout duplicate stderr:\n"
                      << duplicate_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_process.exit_code == 0,
               "#1619: live edited report/label layout object duplicate should exit successfully");
        expect(visual_object_count(asset_path) == before_count + 1U,
               "#1619: live edited report/label layout object duplicate should append one object record");
        expect(visual_object_exists(asset_path, "middle-edited-copy-guid"),
               "#1619: live edited report/label layout object duplicate should persist replacement unique ids");
        expect(visual_object_property(asset_path, "middle-edited-copy-guid", "HPOS") == "300" &&
                   visual_object_property(asset_path, "middle-edited-copy-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "middle-edited-copy-guid", "HEIGHT") == "500",
               "#1619: live edited report/label layout object duplicate should preserve edited geometry fields");
        expect_contains(duplicate_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1619: live edited report/label layout object duplicate should return refreshed report-layout JSON");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1619: live edited report/label layout object duplicate should preserve selected-object availability");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1619: live edited report/label layout object duplicate should preserve containing-section availability");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1619: live edited report/label layout object duplicate should serialize containing-section metadata");
        expect_contains_in_order(
            duplicate_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1100",
                "\"sectionObjectIndex\": 3",
                "\"sectionObjectCount\": 4",
                "\"objectKind\": \"field\"",
                "\"left\": 300",
                "\"top\": 2600",
                "\"width\": 250",
                "\"right\": 550",
                "\"height\": 500",
                "\"bottom\": 3100",
                "\"expression\": \"middle.value\""
            },
            "#1619: live edited report/label layout object duplicate should refresh selected duplicate geometry and section metadata");
    };

    run_live_edited_duplicate(temp_root / "duplicate_live_edited.frx",
                              "duplicate_live_edited.frx",
                              "report");
    run_live_edited_duplicate(temp_root / "duplicate_live_edited.lbx",
                              "duplicate_live_edited.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_live_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_duplicate_live_edited_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_unplaced_duplicate = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " duplicate unplaced live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " duplicate unplaced live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1620: live unplaced report/label layout object duplicate geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1620: live unplaced report/label layout object duplicate geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto duplicate_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--duplicate-object",
                "--unique-id", "middle-field-guid",
                "--new-unique-id", "middle-offband-guid",
                "--json"
            },
            temp_root);

        if (duplicate_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited unplaced layout duplicate stdout:\n"
                      << duplicate_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited unplaced layout duplicate stderr:\n"
                      << duplicate_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_process.exit_code == 0,
               "#1620: live edited unplaced report/label layout object duplicate should exit successfully");
        expect(visual_object_count(asset_path) == before_count + 1U,
               "#1620: live edited unplaced report/label layout object duplicate should append one object record");
        expect(visual_object_exists(asset_path, "middle-offband-guid"),
               "#1620: live edited unplaced report/label layout object duplicate should persist replacement unique ids");
        expect(visual_object_property(asset_path, "middle-offband-guid", "HPOS") == "-300" &&
                   visual_object_property(asset_path, "middle-offband-guid", "VPOS") == "9000" &&
                   visual_object_property(asset_path, "middle-offband-guid", "HEIGHT") == "700",
               "#1620: live edited unplaced report/label layout object duplicate should preserve edited geometry fields");
        expect_contains(duplicate_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1620: live edited unplaced report/label layout object duplicate should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(duplicate_process.stdout_text, "\"isLabel\": true",
                            "#1881: live edited unplaced label layout object duplicate should retain label identity");
        }
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1881: live edited unplaced report/label layout object duplicate should keep preview bounds available");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1881: live edited unplaced report/label layout object duplicate should expand preview left bounds");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#1881: live edited unplaced report/label layout object duplicate should preserve section-origin preview top bounds");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1881: live edited unplaced report/label layout object duplicate should preserve preview right bounds");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1881: live edited unplaced report/label layout object duplicate should expand preview bottom bounds");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsWidth\": 450",
                        "#1881: live edited unplaced report/label layout object duplicate should refresh preview widths");
        expect_contains(duplicate_process.stdout_text, "\"previewBoundsHeight\": 7700",
                        "#1881: live edited unplaced report/label layout object duplicate should refresh preview heights");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1620: live edited unplaced report/label layout object duplicate should preserve selected-object availability");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1620: live edited unplaced report/label layout object duplicate should keep containing-section unavailable");
        expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1620: live edited unplaced report/label layout object duplicate should serialize null containing section");
        expect_contains_in_order(
            duplicate_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 50",
                "\"right\": -250",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1620: live edited unplaced report/label layout object duplicate should refresh selected duplicate geometry without fabricated section metadata");
    };

    run_live_edited_unplaced_duplicate(temp_root / "duplicate_live_edited_unplaced.frx",
                                       "duplicate_live_edited_unplaced.frx",
                                       "report");
    run_live_edited_unplaced_duplicate(temp_root / "duplicate_live_edited_unplaced.lbx",
                                       "duplicate_live_edited_unplaced.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_live_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_rename_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_rename = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " rename live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " rename live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1621: live report/label layout object rename geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1621: live report/label layout object rename geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto rename_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--rename-object",
                "--unique-id", "middle-field-guid",
                "--new-unique-id", "middle-renamed-guid",
                "--json"
            },
            temp_root);

        if (rename_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout rename stdout:\n"
                      << rename_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout rename stderr:\n"
                      << rename_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_process.exit_code == 0,
               "#1621: live edited report/label layout object rename should exit successfully");
        expect(visual_object_count(asset_path) == before_count,
               "#1621: live edited report/label layout object rename should preserve object count");
        expect(!visual_object_exists(asset_path, "middle-field-guid"),
               "#1621: live edited report/label layout object rename should remove the old unique id");
        expect(visual_object_exists(asset_path, "middle-renamed-guid"),
               "#1621: live edited report/label layout object rename should persist replacement unique ids");
        expect(visual_object_property(asset_path, "middle-renamed-guid", "HPOS") == "300" &&
                   visual_object_property(asset_path, "middle-renamed-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "middle-renamed-guid", "HEIGHT") == "500",
               "#1621: live edited report/label layout object rename should preserve edited geometry fields");
        expect_contains(rename_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1621: live edited report/label layout object rename should return refreshed report-layout JSON");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1621: live edited report/label layout object rename should preserve selected-object availability");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1621: live edited report/label layout object rename should preserve containing-section availability");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1621: live edited report/label layout object rename should serialize containing-section metadata");
        expect_contains_in_order(
            rename_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1100",
                "\"sectionObjectIndex\": 2",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": 300",
                "\"top\": 2600",
                "\"width\": 250",
                "\"right\": 550",
                "\"height\": 500",
                "\"bottom\": 3100"
            },
            "#1621: live edited report/label layout object rename should refresh selected renamed geometry and section metadata");
    };

    run_live_edited_rename(temp_root / "rename_live_edited.frx",
                           "rename_live_edited.frx",
                           "report");
    run_live_edited_rename(temp_root / "rename_live_edited.lbx",
                           "rename_live_edited.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_live_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_rename_live_edited_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_unplaced_rename = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "middle-field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " rename unplaced live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " rename unplaced live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1622: live unplaced report/label layout object rename geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "middle-field-guid"),
                   "#1622: live unplaced report/label layout object rename geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto rename_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--rename-object",
                "--unique-id", "middle-field-guid",
                "--new-unique-id", "middle-offband-guid",
                "--json"
            },
            temp_root);

        if (rename_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited unplaced layout rename stdout:\n"
                      << rename_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited unplaced layout rename stderr:\n"
                      << rename_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_process.exit_code == 0,
               "#1622: live edited unplaced report/label layout object rename should exit successfully");
        expect(visual_object_count(asset_path) == before_count,
               "#1622: live edited unplaced report/label layout object rename should preserve object count");
        expect(!visual_object_exists(asset_path, "middle-field-guid"),
               "#1622: live edited unplaced report/label layout object rename should remove the old unique id");
        expect(visual_object_exists(asset_path, "middle-offband-guid"),
               "#1622: live edited unplaced report/label layout object rename should persist replacement unique ids");
        expect(visual_object_property(asset_path, "middle-offband-guid", "HPOS") == "-300" &&
                   visual_object_property(asset_path, "middle-offband-guid", "VPOS") == "9000" &&
                   visual_object_property(asset_path, "middle-offband-guid", "HEIGHT") == "700",
               "#1622: live edited unplaced report/label layout object rename should preserve edited geometry fields");
        expect_contains(rename_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1622: live edited unplaced report/label layout object rename should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(rename_process.stdout_text, "\"isLabel\": true",
                            "#1882: live edited unplaced label layout object rename should retain label identity");
        }
        expect_contains(rename_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1882: live edited unplaced report/label layout object rename should keep preview bounds available");
        expect_contains(rename_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1882: live edited unplaced report/label layout object rename should expand preview left bounds");
        expect_contains(rename_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#1882: live edited unplaced report/label layout object rename should preserve section-origin preview top bounds");
        expect_contains(rename_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1882: live edited unplaced report/label layout object rename should preserve preview right bounds");
        expect_contains(rename_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1882: live edited unplaced report/label layout object rename should expand preview bottom bounds");
        expect_contains(rename_process.stdout_text, "\"previewBoundsWidth\": 450",
                        "#1882: live edited unplaced report/label layout object rename should refresh preview widths");
        expect_contains(rename_process.stdout_text, "\"previewBoundsHeight\": 7700",
                        "#1882: live edited unplaced report/label layout object rename should refresh preview heights");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1622: live edited unplaced report/label layout object rename should preserve selected-object availability");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1622: live edited unplaced report/label layout object rename should keep containing-section unavailable");
        expect_contains(rename_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1622: live edited unplaced report/label layout object rename should serialize null containing section");
        expect_contains_in_order(
            rename_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 50",
                "\"right\": -250",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1622: live edited unplaced report/label layout object rename should refresh selected renamed geometry without fabricated section metadata");
    };

    run_live_edited_unplaced_rename(temp_root / "rename_live_edited_unplaced.frx",
                                    "rename_live_edited_unplaced.frx",
                                    "report");
    run_live_edited_unplaced_rename(temp_root / "rename_live_edited_unplaced.lbx",
                                    "rename_live_edited_unplaced.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_geometry_live_edit_actions <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_nudges_live_edited_report_layout_object_geometry_by_stable_selection(
        argv[1]);
    cf_test_studio_host_json::test_studio_host_json_aligns_live_edited_report_layout_object_geometry_by_stable_selection(
        argv[1]);
    cf_test_studio_host_json::test_studio_host_json_resizes_live_edited_report_layout_object_geometry_by_stable_selection(
        argv[1]);
    cf_test_studio_host_json::test_studio_host_json_snaps_live_edited_report_layout_object_geometry_by_stable_selection(
        argv[1]);
    cf_test_studio_host_json::
        test_studio_host_json_restores_live_edited_then_deleted_report_layout_object_geometry_by_record_selection(
            argv[1]);
    cf_test_studio_host_json::
        test_studio_host_json_restores_live_edited_unplaced_then_deleted_report_layout_object_geometry_by_record_selection(
            argv[1]);
    cf_test_studio_host_json::
        test_studio_host_json_distributes_live_edited_report_layout_object_geometry_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_reorders_live_edited_report_layout_object_geometry_by_record_selection(
        argv[1]);
    cf_test_studio_host_json::
        test_studio_host_json_reorders_live_edited_unplaced_report_layout_object_geometry_by_record_selection(
            argv[1]);
    cf_test_studio_host_json::
        test_studio_host_json_duplicates_live_edited_report_layout_object_geometry_by_record_selection(argv[1]);
    cf_test_studio_host_json::
        test_studio_host_json_duplicates_live_edited_unplaced_report_layout_object_geometry_by_record_selection(
            argv[1]);
    cf_test_studio_host_json::test_studio_host_json_renames_live_edited_report_layout_object_geometry_by_record_selection(
        argv[1]);
    cf_test_studio_host_json::
        test_studio_host_json_renames_live_edited_unplaced_report_layout_object_geometry_by_record_selection(
            argv[1]);

    if (cf_test_studio_host_json::failures != 0) {
        return 1;
    }

    return 0;
}
