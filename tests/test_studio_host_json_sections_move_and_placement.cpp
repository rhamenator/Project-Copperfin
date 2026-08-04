// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_moves_report_layout_objects_from_unplaced_to_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_placement_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_placement_update = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "5",
                "--property-name", "VPOS",
                "--property-value", "2600",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout placement update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout placement update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1531: report/label layout object placement update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 5U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "2600",
               "#1531: report/label layout object placement update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1531: report/label layout object placement update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 3",
                        "#1531: report/label layout object placement update should increment placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1531: report/label layout object placement update should clear unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1531: report/label layout object placement update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1531: report/label layout object placement update should expose selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 700",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"line\""
            },
            "#1531: report/label layout object placement update should refresh selected object section metadata");
    };

    run_placement_update(temp_root / "placement_update.frx", "placement_update.frx", "report");
    run_placement_update(temp_root / "placement_update.lbx", "placement_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_report_layout_objects_from_unplaced_to_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_placement_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_placement_update = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto seed_identity = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 5U,
            .object_name = {},
            .unique_id = {},
            .property_name = "UNIQUEID",
            .property_value = "unplaced-line-guid"
        });
        expect(seed_identity.ok,
               "#1634: report/label layout object stable placement fixture should seed a stable id");

        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "unplaced-line-guid",
                "--property-name", "VPOS",
                "--property-value", "2600",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout placement update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout placement update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1634: report/label layout object stable placement update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 5U,
            .object_name = {},
            .unique_id = "unplaced-line-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "2600",
               "#1634: report/label layout object stable placement update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1634: report/label layout object stable placement update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1634: label layout object stable placement update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 3",
                        "#1634: report/label layout object stable placement update should increment placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1634: report/label layout object stable placement update should clear unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1634: report/label layout object stable placement update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1634: report/label layout object stable placement update should expose selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 700",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"line\"",
                "\"title\": \"unplaced-line-guid\""
            },
            "#1634: report/label layout object stable placement update should refresh selected object section metadata");
    };

    run_placement_update(temp_root / "placement_update_stable.frx",
                         "placement_update_stable.frx",
                         "report");
    run_placement_update(temp_root / "placement_update_stable.lbx",
                         "placement_update_stable.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_report_layout_objects_from_sections_to_unplaced_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_unplacement_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_unplacement_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "VPOS",
                "--property-value", "9000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout unplacement update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout unplacement update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1532: report/label layout object unplacement update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "9000",
               "#1532: report/label layout object unplacement update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1532: report/label layout object unplacement update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1532: report/label layout object unplacement update should decrement placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1532: report/label layout object unplacement update should increment unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1532: report/label layout object unplacement update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1532: report/label layout object unplacement update should clear selected containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1532: report/label layout object unplacement update should serialize null selected containing sections");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\""
            },
            "#1532: report/label layout object unplacement update should refresh selected object unplaced metadata");
    };

    run_unplacement_update(temp_root / "unplacement_update.frx", "unplacement_update.frx", "report");
    run_unplacement_update(temp_root / "unplacement_update.lbx", "unplacement_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_report_layout_objects_from_sections_to_unplaced_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_unplacement_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_unplacement_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "VPOS",
                "--property-value", "9000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout unplacement update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout unplacement update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1635: report/label layout object stable unplacement update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "9000",
               "#1635: report/label layout object stable unplacement update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1635: report/label layout object stable unplacement update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1635: label layout object stable unplacement update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1635: report/label layout object stable unplacement update should decrement placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1635: report/label layout object stable unplacement update should increment unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1635: report/label layout object stable unplacement update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1635: report/label layout object stable unplacement update should clear selected containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1635: report/label layout object stable unplacement update should serialize null selected containing sections");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.company\"",
                "\"top\": 9000",
                "\"bottom\": 9450"
            },
            "#1635: report/label layout object stable unplacement update should refresh selected object unplaced metadata");
    };

    run_unplacement_update(temp_root / "unplacement_update_stable.frx",
                           "unplacement_update_stable.frx",
                           "report");
    run_unplacement_update(temp_root / "unplacement_update_stable.lbx",
                           "unplacement_update_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
