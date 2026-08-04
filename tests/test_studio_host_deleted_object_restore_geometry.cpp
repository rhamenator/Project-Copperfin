// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {

void test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_edited_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_geometry_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1613: restore edited deleted layout object fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "6",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted layout " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted layout " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1613: deleted report/label layout object geometry pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1613: deleted report/label layout object geometry pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "1400");
        set_deleted_geometry("VPOS", "3100");
        set_deleted_geometry("HEIGHT", "900");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "6",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited deleted layout restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited deleted layout restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1613: edited deleted report/label layout object restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1613: edited deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "1400" &&
                   top_property.ok && top_property.exists && top_property.value == "3100" &&
                   height_property.ok && height_property.exists && height_property.value == "900",
               "#1613: edited deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1613: edited deleted report/label layout object restore should return refreshed report-layout JSON");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1613: edited deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1613: edited deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1613: edited deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1613: edited deleted report/label layout object restore should rehydrate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1613: edited deleted report/label layout object restore should serialize containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 2000",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"top\": 3100",
                "\"width\": 1200",
                "\"height\": 900",
                "\"right\": 2600",
                "\"bottom\": 4000"
            },
            "#1613: edited deleted report/label layout object restore should refresh selected live geometry and section metadata");
    };

    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry.frx",
                                 "deleted_restore_edited_geometry.frx",
                                 "report");
    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry.lbx",
                                 "deleted_restore_edited_geometry.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_edited_geometry_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_geometry_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1652: stable restore edited deleted layout object fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-label-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted layout " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted layout " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1652: stable deleted report/label layout object geometry pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1652: stable deleted report/label layout object geometry pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "1400");
        set_deleted_geometry("VPOS", "3100");
        set_deleted_geometry("HEIGHT", "900");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "deleted-label-guid",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable edited deleted layout restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable edited deleted layout restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1652: stable edited deleted report/label layout object restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1652: stable edited deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && !left_property.record_deleted &&
                   left_property.value == "1400" &&
                   top_property.ok && top_property.exists && !top_property.record_deleted &&
                   top_property.value == "3100" &&
                   height_property.ok && height_property.exists && !height_property.record_deleted &&
                   height_property.value == "900",
               "#1652: stable edited deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1652: stable edited deleted report/label layout object restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1652: label stable edited deleted layout object restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1891: stable edited deleted report/label layout object restore should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1891: stable edited deleted report/label layout object restore should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1652: stable edited deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1652: stable edited deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1652: stable edited deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1652: stable edited deleted report/label layout object restore should rehydrate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1652: stable edited deleted report/label layout object restore should serialize containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 2000",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"top\": 3100",
                "\"width\": 1200",
                "\"height\": 900",
                "\"right\": 2600",
                "\"bottom\": 4000"
            },
            "#1652: stable edited deleted report/label layout object restore should refresh selected live geometry and section metadata");
    };

    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry_stable.frx",
                                 "deleted_restore_edited_geometry_stable.frx",
                                 "report");
    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry_stable.lbx",
                                 "deleted_restore_edited_geometry_stable.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_deleted_object_restore_geometry <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_stable_selection(argv[1]);

    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
