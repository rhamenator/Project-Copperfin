// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_LAYOUT_ACTIONS_SKIP_STABLE)
void test_studio_host_json_distributes_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_distribute_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_distribution =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_distribution_json(asset_path);

            const auto distribute_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--unique-id", "detail-header-middle-guid",
                    "--distribute-object",
                    "--distribution-mode", "horizontal",
                    "--distribute-target-unique-id", "detail-header-left-guid",
                    "--distribute-target-unique-id", "detail-header-middle-guid",
                    "--distribute-target-unique-id", "detail-header-right-guid",
                    "--json"
                },
                temp_root);

            if (distribute_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object distribute stdout:\n"
                          << distribute_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object distribute stderr:\n"
                          << distribute_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(distribute_header_process.exit_code == 0,
                   "#1797: detail-header object distribution should exit successfully");
            expect(visual_object_property(asset_path, "detail-header-left-guid", "HPOS") == "100" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "HPOS") == "400" &&
                       visual_object_property(asset_path, "detail-header-right-guid", "HPOS") == "700",
                   "#1797: detail-header object distribution should evenly position the middle object");
            expect(visual_object_property(asset_path, "detail-header-middle-guid", "VPOS") == "60" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "WIDTH") == "50" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "HEIGHT") == "100",
                   "#1797: detail-header object distribution should preserve vertical geometry and size");
            expect_contains(distribute_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1797: detail-header object distribution should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(distribute_header_process.stdout_text, "\"isLabel\": true",
                                "#1797: detail-header label object distribution should retain label identity");
            }
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1797: detail-header object distribution should preserve selected object availability");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1797: detail-header object distribution should preserve object selection kind");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1797: detail-header object distribution should preserve containing-section availability");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2303: detail-header object distribution should preserve live preview availability");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2303: detail-header object distribution should preserve live preview top bounds");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2303: detail-header object distribution should preserve live preview bottom bounds");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2303: detail-header object distribution should preserve live preview heights");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2303: detail-header object distribution should not fabricate deleted preview availability");
            expect_contains(distribute_header_process.stdout_text, "\"dryRun\": false",
                            "#2256: detail-header object distribution JSON should expose committed execution");
            expect_contains(distribute_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2256: detail-header object distribution JSON should expose mutation state");
            expect_contains(distribute_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2256: detail-header object distribution JSON should expose undo availability");
            expect_contains(distribute_header_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                            "#2256: detail-header object distribution JSON should expose horizontal-distribution undo labels");
            expect_contains_in_order(
                distribute_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 2",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 3",
                    "\"objectKind\": \"label\"",
                    "\"left\": 400",
                    "\"top\": 60",
                    "\"width\": 50",
                    "\"right\": 450",
                    "\"height\": 100",
                    "\"bottom\": 160",
                    "\"expression\": \"\\\"Header middle\\\"\""
                },
                "#1797: detail-header object distribution should refresh selected-object section metadata");
            expect_contains_in_order(
                distribute_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 3"
                },
                "#1797: detail-header object distribution should preserve containing-section metadata");

            const auto distribute_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--unique-id", "detail-footer-middle-guid",
                    "--distribute-object",
                    "--distribution-mode", "horizontal",
                    "--distribute-target-unique-id", "detail-footer-left-guid",
                    "--distribute-target-unique-id", "detail-footer-middle-guid",
                    "--distribute-target-unique-id", "detail-footer-right-guid",
                    "--json"
                },
                temp_root);

            if (distribute_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object distribute stdout:\n"
                          << distribute_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object distribute stderr:\n"
                          << distribute_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(distribute_footer_process.exit_code == 0,
                   "#1797: detail-footer object distribution should exit successfully");
            expect(visual_object_property(asset_path, "detail-footer-left-guid", "HPOS") == "140" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "HPOS") == "440" &&
                       visual_object_property(asset_path, "detail-footer-right-guid", "HPOS") == "740",
                   "#1797: detail-footer object distribution should evenly position the middle object");
            expect(visual_object_property(asset_path, "detail-footer-middle-guid", "VPOS") == "370" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "WIDTH") == "50" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "HEIGHT") == "100",
                   "#1797: detail-footer object distribution should preserve vertical geometry and size");
            expect_contains(distribute_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1797: detail-footer object distribution should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(distribute_footer_process.stdout_text, "\"isLabel\": true",
                                "#1797: detail-footer label object distribution should retain label identity");
            }
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1797: detail-footer object distribution should preserve selected object availability");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1797: detail-footer object distribution should preserve object selection kind");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1797: detail-footer object distribution should preserve containing-section availability");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2303: detail-footer object distribution should preserve live preview availability");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2303: detail-footer object distribution should preserve live preview top bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2303: detail-footer object distribution should preserve live preview bottom bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2303: detail-footer object distribution should preserve live preview heights");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2303: detail-footer object distribution should not fabricate deleted preview availability");
            expect_contains(distribute_footer_process.stdout_text, "\"dryRun\": false",
                            "#2256: detail-footer object distribution JSON should expose committed execution");
            expect_contains(distribute_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2256: detail-footer object distribution JSON should expose mutation state");
            expect_contains(distribute_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2256: detail-footer object distribution JSON should expose undo availability");
            expect_contains(distribute_footer_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                            "#2256: detail-footer object distribution JSON should expose horizontal-distribution undo labels");
            expect_contains_in_order(
                distribute_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 6",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 4",
                    "\"sectionRelativeTop\": 70",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 3",
                    "\"objectKind\": \"field\"",
                    "\"left\": 440",
                    "\"top\": 370",
                    "\"width\": 50",
                    "\"right\": 490",
                    "\"height\": 100",
                    "\"bottom\": 470",
                    "\"expression\": \"footer.middle\""
                },
                "#1797: detail-footer object distribution should refresh selected-object section metadata");
            expect_contains_in_order(
                distribute_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 4",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 3"
                },
                "#1797: detail-footer object distribution should preserve containing-section metadata");
        };

    run_detail_header_footer_object_distribution(temp_root / "detail_header_footer_object_distribution.frx",
                                                 "detail_header_footer_object_distribution.frx",
                                                 "report");
    run_detail_header_footer_object_distribution(temp_root / "detail_header_footer_object_distribution.lbx",
                                                 "detail_header_footer_object_distribution.lbx",
                                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_distribute_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_distribution =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_distribution_json(asset_path);

            for (const std::size_t record_index : {1U, 2U, 3U, 5U, 6U, 7U}) {
                const auto delete_result =
                    copperfin::vfp::set_record_deleted_flag(asset_path.string(), record_index, true);
                expect(delete_result.ok && dbf_record_deleted(asset_path, record_index),
                       "#1798: deleted detail header/footer object distribution fixture should mark object records deleted");
            }

            const auto distribute_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--unique-id", "detail-header-middle-guid",
                    "--distribute-object",
                    "--distribution-mode", "horizontal",
                    "--distribute-target-unique-id", "detail-header-left-guid",
                    "--distribute-target-unique-id", "detail-header-middle-guid",
                    "--distribute-target-unique-id", "detail-header-right-guid",
                    "--json"
                },
                temp_root);

            if (distribute_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object distribute stdout:\n"
                          << distribute_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object distribute stderr:\n"
                          << distribute_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(distribute_header_process.exit_code == 0,
                   "#1798: deleted detail-header object distribution should exit successfully");
            expect(visual_object_deleted(asset_path, "detail-header-middle-guid"),
                   "#1798: deleted detail-header object distribution should preserve deleted state");
            expect(visual_object_property(asset_path, "detail-header-left-guid", "HPOS") == "100" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "HPOS") == "400" &&
                       visual_object_property(asset_path, "detail-header-right-guid", "HPOS") == "700",
                   "#1798: deleted detail-header object distribution should evenly position the middle object");
            expect(visual_object_property(asset_path, "detail-header-middle-guid", "VPOS") == "60" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "WIDTH") == "50" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "HEIGHT") == "100",
                   "#1798: deleted detail-header object distribution should preserve vertical geometry and size");
            expect_contains(distribute_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1798: deleted detail-header object distribution should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(distribute_header_process.stdout_text, "\"isLabel\": true",
                                "#1798: deleted detail-header label object distribution should retain label identity");
            }
            expect_contains(distribute_header_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1798: deleted detail-header object distribution should leave live object counts unchanged");
            expect_contains(distribute_header_process.stdout_text, "\"deletedObjectCount\": 6",
                            "#1798: deleted detail-header object distribution should preserve deleted object counts");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1798: deleted detail-header object distribution should preserve selected object availability");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1798: deleted detail-header object distribution should preserve object selection kind");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1798: deleted detail-header object distribution should preserve containing sections");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2304: deleted detail-header object distribution should preserve live preview availability");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2304: deleted detail-header object distribution should preserve live preview top bounds");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2304: deleted detail-header object distribution should preserve live preview bottom bounds");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2304: deleted detail-header object distribution should preserve live preview heights");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2304: deleted detail-header object distribution should expose deleted preview availability");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2304: deleted detail-header object distribution should preserve deleted preview top bounds");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 480",
                            "#2304: deleted detail-header object distribution should preserve deleted preview bottom bounds");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 430",
                            "#2304: deleted detail-header object distribution should preserve deleted preview heights");
            expect_contains(distribute_header_process.stdout_text, "\"dryRun\": false",
                            "#2257: deleted detail-header object distribution JSON should expose committed execution");
            expect_contains(distribute_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2257: deleted detail-header object distribution JSON should expose mutation state");
            expect_contains(distribute_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2257: deleted detail-header object distribution JSON should expose undo availability");
            expect_contains(distribute_header_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                            "#2257: deleted detail-header object distribution JSON should expose horizontal-distribution undo labels");
            expect_contains_in_order(
                distribute_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 3",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Header middle\\\"\"",
                    "\"left\": 400",
                    "\"top\": 60",
                    "\"width\": 50",
                    "\"right\": 450",
                    "\"height\": 100",
                    "\"bottom\": 160"
                },
                "#1798: deleted detail-header object distribution should refresh selected deleted-object metadata");
            expect_contains_in_order(
                distribute_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 3"
                },
                "#1798: deleted detail-header object distribution should preserve containing-section metadata");

            const auto distribute_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--unique-id", "detail-footer-middle-guid",
                    "--distribute-object",
                    "--distribution-mode", "horizontal",
                    "--distribute-target-unique-id", "detail-footer-left-guid",
                    "--distribute-target-unique-id", "detail-footer-middle-guid",
                    "--distribute-target-unique-id", "detail-footer-right-guid",
                    "--json"
                },
                temp_root);

            if (distribute_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object distribute stdout:\n"
                          << distribute_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object distribute stderr:\n"
                          << distribute_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(distribute_footer_process.exit_code == 0,
                   "#1798: deleted detail-footer object distribution should exit successfully");
            expect(visual_object_deleted(asset_path, "detail-footer-middle-guid"),
                   "#1798: deleted detail-footer object distribution should preserve deleted state");
            expect(visual_object_property(asset_path, "detail-footer-left-guid", "HPOS") == "140" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "HPOS") == "440" &&
                       visual_object_property(asset_path, "detail-footer-right-guid", "HPOS") == "740",
                   "#1798: deleted detail-footer object distribution should evenly position the middle object");
            expect(visual_object_property(asset_path, "detail-footer-middle-guid", "VPOS") == "370" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "WIDTH") == "50" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "HEIGHT") == "100",
                   "#1798: deleted detail-footer object distribution should preserve vertical geometry and size");
            expect_contains(distribute_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1798: deleted detail-footer object distribution should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(distribute_footer_process.stdout_text, "\"isLabel\": true",
                                "#1798: deleted detail-footer label object distribution should retain label identity");
            }
            expect_contains(distribute_footer_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1798: deleted detail-footer object distribution should leave live object counts unchanged");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedObjectCount\": 6",
                            "#1798: deleted detail-footer object distribution should preserve deleted object counts");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1798: deleted detail-footer object distribution should preserve selected object availability");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1798: deleted detail-footer object distribution should preserve object selection kind");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1798: deleted detail-footer object distribution should preserve containing sections");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2304: deleted detail-footer object distribution should preserve live preview availability");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2304: deleted detail-footer object distribution should preserve live preview top bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2304: deleted detail-footer object distribution should preserve live preview bottom bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2304: deleted detail-footer object distribution should preserve live preview heights");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2304: deleted detail-footer object distribution should expose deleted preview availability");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2304: deleted detail-footer object distribution should preserve deleted preview top bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 480",
                            "#2304: deleted detail-footer object distribution should preserve deleted preview bottom bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 430",
                            "#2304: deleted detail-footer object distribution should preserve deleted preview heights");
            expect_contains(distribute_footer_process.stdout_text, "\"dryRun\": false",
                            "#2257: deleted detail-footer object distribution JSON should expose committed execution");
            expect_contains(distribute_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2257: deleted detail-footer object distribution JSON should expose mutation state");
            expect_contains(distribute_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2257: deleted detail-footer object distribution JSON should expose undo availability");
            expect_contains(distribute_footer_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                            "#2257: deleted detail-footer object distribution JSON should expose horizontal-distribution undo labels");
            expect_contains_in_order(
                distribute_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 6",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 4",
                    "\"sectionRelativeTop\": 70",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 3",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"footer.middle\"",
                    "\"left\": 440",
                    "\"top\": 370",
                    "\"width\": 50",
                    "\"right\": 490",
                    "\"height\": 100",
                    "\"bottom\": 470"
                },
                "#1798: deleted detail-footer object distribution should refresh selected deleted-object metadata");
            expect_contains_in_order(
                distribute_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 4",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 3"
                },
                "#1798: deleted detail-footer object distribution should preserve containing-section metadata");
        };

    run_deleted_detail_header_footer_object_distribution(
        temp_root / "deleted_detail_header_footer_object_distribution.frx",
        "deleted_detail_header_footer_object_distribution.frx",
        "report");
    run_deleted_detail_header_footer_object_distribution(
        temp_root / "deleted_detail_header_footer_object_distribution.lbx",
        "deleted_detail_header_footer_object_distribution.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_detail_header_footer_objects_vertically_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_vertical_distribute_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_vertical_distribution =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_vertical_distribution_json(asset_path);

            const auto distribute_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--unique-id", "detail-header-middle-guid",
                    "--distribute-object",
                    "--distribution-mode", "vertical",
                    "--distribute-target-unique-id", "detail-header-left-guid",
                    "--distribute-target-unique-id", "detail-header-middle-guid",
                    "--distribute-target-unique-id", "detail-header-right-guid",
                    "--json"
                },
                temp_root);

            if (distribute_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object vertical distribute stdout:\n"
                          << distribute_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object vertical distribute stderr:\n"
                          << distribute_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(distribute_header_process.exit_code == 0,
                   "#1799: detail-header object vertical distribution should exit successfully");
            expect(visual_object_property(asset_path, "detail-header-left-guid", "VPOS") == "40" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "VPOS") == "65" &&
                       visual_object_property(asset_path, "detail-header-right-guid", "VPOS") == "90",
                   "#1799: detail-header object vertical distribution should evenly position the middle object");
            expect(visual_object_property(asset_path, "detail-header-middle-guid", "HPOS") == "175" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "WIDTH") == "50" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "HEIGHT") == "100",
                   "#1799: detail-header object vertical distribution should preserve horizontal geometry and size");
            expect_contains(distribute_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1799: detail-header object vertical distribution should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(distribute_header_process.stdout_text, "\"isLabel\": true",
                                "#1799: detail-header label object vertical distribution should retain label identity");
            }
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1799: detail-header object vertical distribution should preserve selected object availability");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1799: detail-header object vertical distribution should preserve object selection kind");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1799: detail-header object vertical distribution should preserve containing-section availability");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2305: detail-header object vertical distribution should preserve live preview availability");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2305: detail-header object vertical distribution should preserve live preview top bounds");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2305: detail-header object vertical distribution should preserve live preview bottom bounds");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2305: detail-header object vertical distribution should preserve live preview heights");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2305: detail-header object vertical distribution should not fabricate deleted preview bounds");
            expect_contains(distribute_header_process.stdout_text, "\"dryRun\": false",
                            "#2258: detail-header object vertical distribution JSON should expose committed execution");
            expect_contains(distribute_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2258: detail-header object vertical distribution JSON should expose mutation state");
            expect_contains(distribute_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2258: detail-header object vertical distribution JSON should expose undo availability");
            expect_contains(distribute_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2258: detail-header object vertical distribution JSON should expose vertical-distribution undo labels");
            expect_contains_in_order(
                distribute_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 2",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 65",
                    "\"sectionRelativeBottom\": 165",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 3",
                    "\"objectKind\": \"label\"",
                    "\"left\": 175",
                    "\"top\": 65",
                    "\"width\": 50",
                    "\"right\": 225",
                    "\"height\": 100",
                    "\"bottom\": 165",
                    "\"expression\": \"\\\"Header middle\\\"\""
                },
                "#1799: detail-header object vertical distribution should refresh selected-object section metadata");
            expect_contains_in_order(
                distribute_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 3"
                },
                "#1799: detail-header object vertical distribution should preserve containing-section metadata");

            const auto distribute_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--unique-id", "detail-footer-middle-guid",
                    "--distribute-object",
                    "--distribution-mode", "vertical",
                    "--distribute-target-unique-id", "detail-footer-left-guid",
                    "--distribute-target-unique-id", "detail-footer-middle-guid",
                    "--distribute-target-unique-id", "detail-footer-right-guid",
                    "--json"
                },
                temp_root);

            if (distribute_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object vertical distribute stdout:\n"
                          << distribute_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object vertical distribute stderr:\n"
                          << distribute_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(distribute_footer_process.exit_code == 0,
                   "#1799: detail-footer object vertical distribution should exit successfully");
            expect(visual_object_property(asset_path, "detail-footer-left-guid", "VPOS") == "330" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "VPOS") == "360" &&
                       visual_object_property(asset_path, "detail-footer-right-guid", "VPOS") == "390",
                   "#1799: detail-footer object vertical distribution should evenly position the middle object");
            expect(visual_object_property(asset_path, "detail-footer-middle-guid", "HPOS") == "200" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "WIDTH") == "50" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "HEIGHT") == "100",
                   "#1799: detail-footer object vertical distribution should preserve horizontal geometry and size");
            expect_contains(distribute_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1799: detail-footer object vertical distribution should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(distribute_footer_process.stdout_text, "\"isLabel\": true",
                                "#1799: detail-footer label object vertical distribution should retain label identity");
            }
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1799: detail-footer object vertical distribution should preserve selected object availability");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1799: detail-footer object vertical distribution should preserve object selection kind");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1799: detail-footer object vertical distribution should preserve containing-section availability");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2305: detail-footer object vertical distribution should preserve live preview availability");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2305: detail-footer object vertical distribution should preserve live preview top bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2305: detail-footer object vertical distribution should preserve live preview bottom bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2305: detail-footer object vertical distribution should preserve live preview heights");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2305: detail-footer object vertical distribution should not fabricate deleted preview bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"dryRun\": false",
                            "#2258: detail-footer object vertical distribution JSON should expose committed execution");
            expect_contains(distribute_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2258: detail-footer object vertical distribution JSON should expose mutation state");
            expect_contains(distribute_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2258: detail-footer object vertical distribution JSON should expose undo availability");
            expect_contains(distribute_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2258: detail-footer object vertical distribution JSON should expose vertical-distribution undo labels");
            expect_contains_in_order(
                distribute_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 6",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 4",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 3",
                    "\"objectKind\": \"field\"",
                    "\"left\": 200",
                    "\"top\": 360",
                    "\"width\": 50",
                    "\"right\": 250",
                    "\"height\": 100",
                    "\"bottom\": 460",
                    "\"expression\": \"footer.middle\""
                },
                "#1799: detail-footer object vertical distribution should refresh selected-object section metadata");
            expect_contains_in_order(
                distribute_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 4",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 3"
                },
                "#1799: detail-footer object vertical distribution should preserve containing-section metadata");
        };

    run_detail_header_footer_object_vertical_distribution(
        temp_root / "detail_header_footer_object_vertical_distribution.frx",
        "detail_header_footer_object_vertical_distribution.frx",
        "report");
    run_detail_header_footer_object_vertical_distribution(
        temp_root / "detail_header_footer_object_vertical_distribution.lbx",
        "detail_header_footer_object_vertical_distribution.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_deleted_detail_header_footer_objects_vertically_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root = fs::temp_directory_path() /
                               "copperfin_studio_host_deleted_detail_header_footer_object_vertical_distribute_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_vertical_distribution =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_vertical_distribution_json(asset_path);

            for (const std::size_t record_index : {1U, 2U, 3U, 5U, 6U, 7U}) {
                const auto delete_result =
                    copperfin::vfp::set_record_deleted_flag(asset_path.string(), record_index, true);
                expect(delete_result.ok && dbf_record_deleted(asset_path, record_index),
                       "#1800: deleted detail header/footer object vertical distribution fixture should mark object records deleted");
            }

            const auto distribute_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--unique-id", "detail-header-middle-guid",
                    "--distribute-object",
                    "--distribution-mode", "vertical",
                    "--distribute-target-unique-id", "detail-header-left-guid",
                    "--distribute-target-unique-id", "detail-header-middle-guid",
                    "--distribute-target-unique-id", "detail-header-right-guid",
                    "--json"
                },
                temp_root);

            if (distribute_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object vertical distribute stdout:\n"
                          << distribute_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object vertical distribute stderr:\n"
                          << distribute_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(distribute_header_process.exit_code == 0,
                   "#1800: deleted detail-header object vertical distribution should exit successfully");
            expect(visual_object_deleted(asset_path, "detail-header-middle-guid"),
                   "#1800: deleted detail-header object vertical distribution should preserve deleted state");
            expect(visual_object_property(asset_path, "detail-header-left-guid", "VPOS") == "40" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "VPOS") == "65" &&
                       visual_object_property(asset_path, "detail-header-right-guid", "VPOS") == "90",
                   "#1800: deleted detail-header object vertical distribution should evenly position the middle object");
            expect(visual_object_property(asset_path, "detail-header-middle-guid", "HPOS") == "175" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "WIDTH") == "50" &&
                       visual_object_property(asset_path, "detail-header-middle-guid", "HEIGHT") == "100",
                   "#1800: deleted detail-header object vertical distribution should preserve horizontal geometry and size");
            expect_contains(distribute_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1800: deleted detail-header object vertical distribution should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(distribute_header_process.stdout_text, "\"isLabel\": true",
                                "#1800: deleted detail-header label object vertical distribution should retain label identity");
            }
            expect_contains(distribute_header_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1800: deleted detail-header object vertical distribution should leave live object counts unchanged");
            expect_contains(distribute_header_process.stdout_text, "\"deletedObjectCount\": 6",
                            "#1800: deleted detail-header object vertical distribution should preserve deleted object counts");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1800: deleted detail-header object vertical distribution should preserve selected object availability");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1800: deleted detail-header object vertical distribution should preserve object selection kind");
            expect_contains(distribute_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1800: deleted detail-header object vertical distribution should preserve containing sections");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2306: deleted detail-header object vertical distribution should preserve live preview availability");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2306: deleted detail-header object vertical distribution should preserve live preview top bounds");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2306: deleted detail-header object vertical distribution should preserve live preview bottom bounds");
            expect_contains(distribute_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2306: deleted detail-header object vertical distribution should preserve live preview heights");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2306: deleted detail-header object vertical distribution should expose deleted preview availability");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 40",
                            "#2306: deleted detail-header object vertical distribution should preserve deleted preview top bounds");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 490",
                            "#2306: deleted detail-header object vertical distribution should preserve deleted preview bottom bounds");
            expect_contains(distribute_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 450",
                            "#2306: deleted detail-header object vertical distribution should preserve deleted preview heights");
            expect_contains(distribute_header_process.stdout_text, "\"dryRun\": false",
                            "#2259: deleted detail-header object vertical distribution JSON should expose committed execution");
            expect_contains(distribute_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2259: deleted detail-header object vertical distribution JSON should expose mutation state");
            expect_contains(distribute_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2259: deleted detail-header object vertical distribution JSON should expose undo availability");
            expect_contains(distribute_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2259: deleted detail-header object vertical distribution JSON should expose vertical-distribution undo labels");
            expect_contains_in_order(
                distribute_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 65",
                    "\"sectionRelativeBottom\": 165",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 3",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Header middle\\\"\"",
                    "\"left\": 175",
                    "\"top\": 65",
                    "\"width\": 50",
                    "\"right\": 225",
                    "\"height\": 100",
                    "\"bottom\": 165"
                },
                "#1800: deleted detail-header object vertical distribution should refresh selected deleted-object metadata");
            expect_contains_in_order(
                distribute_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 3"
                },
                "#1800: deleted detail-header object vertical distribution should preserve containing-section metadata");

            const auto distribute_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--unique-id", "detail-footer-middle-guid",
                    "--distribute-object",
                    "--distribution-mode", "vertical",
                    "--distribute-target-unique-id", "detail-footer-left-guid",
                    "--distribute-target-unique-id", "detail-footer-middle-guid",
                    "--distribute-target-unique-id", "detail-footer-right-guid",
                    "--json"
                },
                temp_root);

            if (distribute_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object vertical distribute stdout:\n"
                          << distribute_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object vertical distribute stderr:\n"
                          << distribute_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(distribute_footer_process.exit_code == 0,
                   "#1800: deleted detail-footer object vertical distribution should exit successfully");
            expect(visual_object_deleted(asset_path, "detail-footer-middle-guid"),
                   "#1800: deleted detail-footer object vertical distribution should preserve deleted state");
            expect(visual_object_property(asset_path, "detail-footer-left-guid", "VPOS") == "330" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "VPOS") == "360" &&
                       visual_object_property(asset_path, "detail-footer-right-guid", "VPOS") == "390",
                   "#1800: deleted detail-footer object vertical distribution should evenly position the middle object");
            expect(visual_object_property(asset_path, "detail-footer-middle-guid", "HPOS") == "200" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "WIDTH") == "50" &&
                       visual_object_property(asset_path, "detail-footer-middle-guid", "HEIGHT") == "100",
                   "#1800: deleted detail-footer object vertical distribution should preserve horizontal geometry and size");
            expect_contains(distribute_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1800: deleted detail-footer object vertical distribution should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(distribute_footer_process.stdout_text, "\"isLabel\": true",
                                "#1800: deleted detail-footer label object vertical distribution should retain label identity");
            }
            expect_contains(distribute_footer_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1800: deleted detail-footer object vertical distribution should leave live object counts unchanged");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedObjectCount\": 6",
                            "#1800: deleted detail-footer object vertical distribution should preserve deleted object counts");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1800: deleted detail-footer object vertical distribution should preserve selected object availability");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1800: deleted detail-footer object vertical distribution should preserve object selection kind");
            expect_contains(distribute_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1800: deleted detail-footer object vertical distribution should preserve containing sections");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2306: deleted detail-footer object vertical distribution should preserve live preview availability");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2306: deleted detail-footer object vertical distribution should preserve live preview top bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2306: deleted detail-footer object vertical distribution should preserve live preview bottom bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2306: deleted detail-footer object vertical distribution should preserve live preview heights");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2306: deleted detail-footer object vertical distribution should expose deleted preview availability");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 40",
                            "#2306: deleted detail-footer object vertical distribution should preserve deleted preview top bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 490",
                            "#2306: deleted detail-footer object vertical distribution should preserve deleted preview bottom bounds");
            expect_contains(distribute_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 450",
                            "#2306: deleted detail-footer object vertical distribution should preserve deleted preview heights");
            expect_contains(distribute_footer_process.stdout_text, "\"dryRun\": false",
                            "#2259: deleted detail-footer object vertical distribution JSON should expose committed execution");
            expect_contains(distribute_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2259: deleted detail-footer object vertical distribution JSON should expose mutation state");
            expect_contains(distribute_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2259: deleted detail-footer object vertical distribution JSON should expose undo availability");
            expect_contains(distribute_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2259: deleted detail-footer object vertical distribution JSON should expose vertical-distribution undo labels");
            expect_contains_in_order(
                distribute_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 6",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 4",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 3",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"footer.middle\"",
                    "\"left\": 200",
                    "\"top\": 360",
                    "\"width\": 50",
                    "\"right\": 250",
                    "\"height\": 100",
                    "\"bottom\": 460"
                },
                "#1800: deleted detail-footer object vertical distribution should refresh selected deleted-object metadata");
            expect_contains_in_order(
                distribute_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 4",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 3"
                },
                "#1800: deleted detail-footer object vertical distribution should preserve containing-section metadata");
        };

    run_deleted_detail_header_footer_object_vertical_distribution(
        temp_root / "deleted_detail_header_footer_object_vertical_distribution.frx",
        "deleted_detail_header_footer_object_vertical_distribution.frx",
        "report");
    run_deleted_detail_header_footer_object_vertical_distribution(
        temp_root / "deleted_detail_header_footer_object_vertical_distribution.lbx",
        "deleted_detail_header_footer_object_vertical_distribution.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif
}  // namespace cf_test_studio_host_json
