// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {

void test_studio_host_json_deletes_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_delete_edited_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_edited_geometry_delete = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1615: edited report/label layout object delete fixture should start live");

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
                std::cerr << "studio host " << label << " live " << property_name
                          << " pre-delete update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live " << property_name
                          << " pre-delete update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1615: report/label layout object geometry pre-delete update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1615: report/label layout object geometry pre-delete update should preserve live state");
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
            std::cerr << "studio host " << label << " edited layout delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1615: edited report/label layout object delete should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1615: edited report/label layout object delete should mark the DBF record deleted");
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
               "#1615: edited report/label layout object delete should preserve edited geometry fields");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1615: edited report/label layout object delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1887: edited label layout object delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1887: edited report/label layout object delete should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1887: edited report/label layout object delete should refresh live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1887: edited report/label layout object delete should refresh live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1887: edited report/label layout object delete should refresh live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1887: edited report/label layout object delete should refresh live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1887: edited report/label layout object delete should refresh live preview widths");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1887: edited report/label layout object delete should refresh live preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1887: edited report/label layout object delete should preserve deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1887: edited report/label layout object delete should refresh deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1887: edited report/label layout object delete should refresh deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 3800",
                        "#1887: edited report/label layout object delete should refresh deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3500",
                        "#1887: edited report/label layout object delete should refresh deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2800",
                        "#1887: edited report/label layout object delete should refresh deleted preview widths");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 900",
                        "#1887: edited report/label layout object delete should refresh deleted preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1615: edited report/label layout object delete should add the edited object to deleted-object counts");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1615: edited report/label layout object delete should preserve selected deleted-object availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1615: edited report/label layout object delete should preserve object selection kind");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1615: edited report/label layout object delete should preserve containing-section availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1615: edited report/label layout object delete should serialize containing-section metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1500",
                "\"objectKind\": \"field\"",
                "\"left\": 1400",
                "\"top\": 2600",
                "\"width\": 2400",
                "\"right\": 3800",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1615: edited report/label layout object delete should preserve edited deleted-object geometry metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1500",
                "\"objectKind\": \"field\"",
                "\"left\": 1400",
                "\"top\": 2600",
                "\"width\": 2400",
                "\"right\": 3800",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1615: edited report/label layout object delete should preserve selected deleted-object geometry metadata");
    };

    run_edited_geometry_delete(temp_root / "delete_edited_geometry.frx",
                               "delete_edited_geometry.frx",
                               "report");
    run_edited_geometry_delete(temp_root / "delete_edited_geometry.lbx",
                               "delete_edited_geometry.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_delete_edited_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_edited_unplaced_delete = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1616: edited unplaced report/label layout object delete fixture should start live");

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
                std::cerr << "studio host " << label << " live unplaced " << property_name
                          << " pre-delete update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live unplaced " << property_name
                          << " pre-delete update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1616: report/label layout object unplaced pre-delete update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1616: report/label layout object unplaced pre-delete update should preserve live state");
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
            std::cerr << "studio host " << label << " edited unplaced layout delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited unplaced layout delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1616: edited unplaced report/label layout object delete should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1616: edited unplaced report/label layout object delete should mark the DBF record deleted");
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
               "#1616: edited unplaced report/label layout object delete should preserve edited geometry fields");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1616: edited unplaced report/label layout object delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1889: edited unplaced label layout object delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1889: edited unplaced report/label layout object delete should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview widths");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1616: edited unplaced report/label layout object delete should keep deleted preview bounds available");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": -300",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1889: edited unplaced report/label layout object delete should preserve deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 3700",
                        "#1889: edited unplaced report/label layout object delete should expand deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 9700",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 4000",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview widths");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 7100",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1616: edited unplaced report/label layout object delete should add the edited object to deleted-object counts");
        expect_contains(delete_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1616: edited unplaced report/label layout object delete should preserve deleted placed-object counts");
        expect_contains(delete_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                        "#1616: edited unplaced report/label layout object delete should count the edited deleted object as unplaced");
        expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1616: edited unplaced report/label layout object delete should remove the edited object from live unplaced counts");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1616: edited unplaced report/label layout object delete should preserve selected deleted-object availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1616: edited unplaced report/label layout object delete should preserve object selection kind");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1616: edited unplaced report/label layout object delete should not fabricate containing-section availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1616: edited unplaced report/label layout object delete should serialize null containing-section metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 4000",
                "\"right\": 3700",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1616: edited unplaced report/label layout object delete should preserve edited deleted-object geometry metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 4000",
                "\"right\": 3700",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1616: edited unplaced report/label layout object delete should preserve selected deleted-object geometry metadata");
    };

    run_edited_unplaced_delete(temp_root / "delete_edited_unplaced.frx",
                               "delete_edited_unplaced.frx",
                               "report");
    run_edited_unplaced_delete(temp_root / "delete_edited_unplaced.lbx",
                               "delete_edited_unplaced.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_edited_object_delete_geometry <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_deletes_edited_report_layout_object_geometry_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_deletes_edited_unplaced_report_layout_object_geometry_by_record_selection(argv[1]);

    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
