// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS)
#if !defined(COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_SKIP_HOST_SMOKE)

void test_studio_host_json_refreshes_deleted_detail_header_footer_section_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_preview_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto update_footer_height_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "700",
                    "--json"
                },
                temp_root);

            if (update_footer_height_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section preview height stdout:\n"
                          << update_footer_height_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section preview height stderr:\n"
                          << update_footer_height_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_height_process.exit_code == 0,
                   "#1820: deleted detail-footer section preview height update by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1820: deleted detail-footer section preview height update should preserve deleted state");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.value == "700",
                   "#1820: deleted detail-footer section preview height update should persist the HEIGHT field");
            expect_contains(update_footer_height_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1820: deleted detail-footer section preview height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_height_process.stdout_text, "\"isLabel\": true",
                                "#1820: deleted detail-footer label section preview height update should retain label identity");
            }
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1820: deleted detail-footer section preview height update should preserve live preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1820: deleted detail-footer section preview height update should preserve live preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve live preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsHeight\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve live preview heights");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1820: deleted detail-footer section preview height update should preserve deleted preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve deleted preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1500",
                            "#1820: deleted detail-footer section preview height update should refresh deleted preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsHeight\": 1000",
                            "#1820: deleted detail-footer section preview height update should refresh deleted preview heights");
            expect_contains(update_footer_height_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve live section height totals");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedSectionHeightTotal\": 1000",
                            "#1820: deleted detail-footer section preview height update should refresh deleted section height totals");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1820: deleted detail-footer section preview height update should preserve selected section availability");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1820: deleted detail-footer section preview height update should preserve selection kind");
            expect_contains_in_order(
                update_footer_height_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"top\": 800",
                    "\"height\": 700",
                    "\"bottom\": 1500"
                },
                "#1820: deleted detail-footer section preview height update should refresh selected deleted-section geometry");

            const auto update_header_top_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "VPOS",
                    "--property-value", "350",
                    "--json"
                },
                temp_root);

            if (update_header_top_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section preview top stdout:\n"
                          << update_header_top_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section preview top stderr:\n"
                          << update_header_top_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_top_process.exit_code == 0,
                   "#1820: deleted detail-header section preview top update by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1820: deleted detail-header section preview top update should preserve deleted state");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.value == "350",
                   "#1820: deleted detail-header section preview top update should persist the VPOS field");
            expect_contains(update_header_top_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1820: deleted detail-header section preview top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_top_process.stdout_text, "\"isLabel\": true",
                                "#1820: deleted detail-header label section preview top update should retain label identity");
            }
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1820: deleted detail-header section preview top update should preserve live preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1820: deleted detail-header section preview top update should preserve live preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1820: deleted detail-header section preview top update should preserve live preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsHeight\": 500",
                            "#1820: deleted detail-header section preview top update should preserve live preview heights");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1820: deleted detail-header section preview top update should preserve deleted preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsTop\": 350",
                            "#1820: deleted detail-header section preview top update should refresh deleted preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1500",
                            "#1820: deleted detail-header section preview top update should preserve expanded deleted preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsHeight\": 1150",
                            "#1820: deleted detail-header section preview top update should refresh deleted preview heights");
            expect_contains(update_header_top_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1820: deleted detail-header section preview top update should preserve live section height totals");
            expect_contains(update_header_top_process.stdout_text, "\"deletedSectionHeightTotal\": 1000",
                            "#1820: deleted detail-header section preview top update should preserve deleted section height totals");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1820: deleted detail-header section preview top update should preserve selected section availability");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1820: deleted detail-header section preview top update should preserve selection kind");
            expect_contains_in_order(
                update_header_top_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"top\": 350",
                    "\"height\": 300",
                    "\"bottom\": 650"
                },
                "#1820: deleted detail-header section preview top update should refresh selected deleted-section geometry");
        };

    run_deleted_detail_header_footer_section_preview_bounds(
        temp_root / "deleted_detail_header_footer_section_preview_bounds_stable.frx",
        "deleted_detail_header_footer_section_preview_bounds_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_preview_bounds(
        temp_root / "deleted_detail_header_footer_section_preview_bounds_stable.lbx",
        "deleted_detail_header_footer_section_preview_bounds_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_refreshes_detail_header_footer_section_delete_restore_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_detail_header_footer_section_delete_restore_preview_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_delete_restore_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto delete_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--delete-object",
                    "--unique-id", "detail-header-guid",
                    "--json"
                },
                temp_root);

            if (delete_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section preview delete stdout:\n"
                          << delete_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section preview delete stderr:\n"
                          << delete_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(delete_process.exit_code == 0,
                   "#2278: detail-header section preview delete by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 0U),
                   "#2278: detail-header section preview delete should mark the section deleted");
            expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#2278: detail-header section preview delete should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                                "#2278: detail-header label section preview delete should retain label identity");
            }
            expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2278: detail-header section preview delete should preserve live preview availability");
            expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                            "#2278: detail-header section preview delete should preserve live preview left bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 50",
                            "#2278: detail-header section preview delete should preserve the retained header-object top bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 1040",
                            "#2278: detail-header section preview delete should preserve live preview right bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2278: detail-header section preview delete should preserve sibling live preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 1040",
                            "#2278: detail-header section preview delete should preserve live preview widths");
            expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 500",
                            "#2278: detail-header section preview delete should preserve the retained header-object preview height");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2278: detail-header section preview delete should expose deleted preview availability");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                            "#2278: detail-header section preview delete should expose deleted preview left bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                            "#2278: detail-header section preview delete should expose deleted preview top bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 800",
                            "#2278: detail-header section preview delete should expose deleted preview right bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 300",
                            "#2278: detail-header section preview delete should expose deleted preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 800",
                            "#2278: detail-header section preview delete should expose deleted preview widths");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                            "#2278: detail-header section preview delete should expose deleted preview heights");
            expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                            "#2278: detail-header section preview delete should keep the sibling section live");
            expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                            "#2278: detail-header section preview delete should expose one deleted section");
            expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 2",
                            "#2278: detail-header section preview delete should keep sibling objects placed");
            expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#2278: detail-header section preview delete should not orphan former header objects");
            expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#2278: detail-header section preview delete should preserve selected section availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#2278: detail-header section preview delete should preserve selection kind");
            expect_contains(delete_process.stdout_text, "\"dryRun\": false",
                            "#2278: detail-header section preview delete JSON should expose committed state");
            expect_contains(delete_process.stdout_text, "\"mutatesAsset\": true",
                            "#2278: detail-header section preview delete JSON should expose mutation state");
            expect_contains(delete_process.stdout_text, "\"undoAvailable\": true",
                            "#2278: detail-header section preview delete JSON should expose undo availability");
            expect_contains(delete_process.stdout_text, "\"undoLabel\": \"Deleted state\"",
                            "#2278: detail-header section preview delete JSON should expose the deleted-state undo label");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300",
                    "\"objectCount\": 1"
                },
                "#2278: detail-header section preview delete should refresh selected deleted-section geometry");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"objects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#2278: detail-header section preview delete should retain former header object containment inside the deleted section");

            const auto restore_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--restore-object",
                    "--unique-id", "detail-header-guid",
                    "--json"
                },
                temp_root);

            if (restore_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section preview restore stdout:\n"
                          << restore_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section preview restore stderr:\n"
                          << restore_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(restore_process.exit_code == 0,
                   "#2278: detail-header section preview restore by stable selection should exit successfully");
            expect(!dbf_record_deleted(asset_path, 0U),
                   "#2278: detail-header section preview restore should clear the deleted state");
            expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#2278: detail-header section preview restore should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                                "#2278: detail-header label section preview restore should retain label identity");
            }
            expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2278: detail-header section preview restore should preserve live preview availability");
            expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2278: detail-header section preview restore should restore live preview top bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2278: detail-header section preview restore should preserve live preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2278: detail-header section preview restore should restore live preview heights");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2278: detail-header section preview restore should clear deleted preview availability");
            expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                            "#2278: detail-header section preview restore should restore live section counts");
            expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                            "#2278: detail-header section preview restore should clear deleted section counts");
            expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                            "#2278: detail-header section preview restore should restore placed object counts");
            expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#2278: detail-header section preview restore should clear unplaced object counts");
            expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#2278: detail-header section preview restore should preserve selected section availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#2278: detail-header section preview restore should preserve selection kind");
            expect_contains(restore_process.stdout_text, "\"dryRun\": false",
                            "#2278: detail-header section preview restore JSON should expose committed state");
            expect_contains(restore_process.stdout_text, "\"mutatesAsset\": true",
                            "#2278: detail-header section preview restore JSON should expose mutation state");
            expect_contains(restore_process.stdout_text, "\"undoAvailable\": true",
                            "#2278: detail-header section preview restore JSON should expose undo availability");
            expect_contains(restore_process.stdout_text, "\"undoLabel\": \"Deleted state\"",
                            "#2278: detail-header section preview restore JSON should expose the deleted-state undo label");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"deleted\": false",
                    "\"sectionIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300"
                },
                "#2278: detail-header section preview restore should refresh selected live-section geometry");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"objects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#2278: detail-header section preview restore should restore header object containment");
        };

    const auto run_detail_footer_delete_restore_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto delete_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--delete-object",
                    "--unique-id", "detail-footer-guid",
                    "--json"
                },
                temp_root);

            if (delete_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section preview delete stdout:\n"
                          << delete_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section preview delete stderr:\n"
                          << delete_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(delete_process.exit_code == 0,
                   "#1821: detail-footer section preview delete by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1821: detail-footer section preview delete should mark the section deleted");
            expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1821: detail-footer section preview delete should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                                "#1821: detail-footer label section preview delete should retain label identity");
            }
            expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1821: detail-footer section preview delete should preserve live preview availability");
            expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                            "#1821: detail-footer section preview delete should preserve live preview left bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1821: detail-footer section preview delete should preserve live preview top bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 1040",
                            "#1821: detail-footer section preview delete should preserve live preview right bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 460",
                            "#1821: detail-footer section preview delete should preserve the retained footer-object bottom bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 1040",
                            "#1821: detail-footer section preview delete should preserve live preview widths");
            expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 460",
                            "#1821: detail-footer section preview delete should preserve the retained footer-object preview height");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1821: detail-footer section preview delete should expose deleted preview availability");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                            "#1821: detail-footer section preview delete should expose deleted preview left bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 300",
                            "#1821: detail-footer section preview delete should expose deleted preview top bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 1040",
                            "#1821: detail-footer section preview delete should expose deleted preview right bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 550",
                            "#1821: detail-footer section preview delete should expose deleted preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1040",
                            "#1821: detail-footer section preview delete should expose deleted preview widths");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                            "#1821: detail-footer section preview delete should expose deleted preview heights");
            expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                            "#1821: detail-footer section preview delete should keep the sibling section live");
            expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                            "#1821: detail-footer section preview delete should expose one deleted section");
            expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 2",
                            "#1821: detail-footer section preview delete should keep sibling objects placed");
            expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#1821: detail-footer section preview delete should not orphan former footer objects");
            expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1821: detail-footer section preview delete should preserve selected section availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1821: detail-footer section preview delete should preserve selection kind");
            expect_contains(delete_process.stdout_text, "\"dryRun\": false",
                            "#2241: detail-footer section preview delete JSON should expose committed state");
            expect_contains(delete_process.stdout_text, "\"mutatesAsset\": true",
                            "#2241: detail-footer section preview delete JSON should expose mutation state");
            expect_contains(delete_process.stdout_text, "\"undoAvailable\": true",
                            "#2241: detail-footer section preview delete JSON should expose undo availability");
            expect_contains(delete_process.stdout_text, "\"undoLabel\": \"Deleted state\"",
                            "#2241: detail-footer section preview delete JSON should expose the deleted-state undo label");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"top\": 300",
                    "\"height\": 250",
                    "\"bottom\": 550",
                    "\"objectCount\": 1"
                },
                "#1821: detail-footer section preview delete should refresh selected deleted-section geometry");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Footer\"",
                    "\"objects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#1821: detail-footer section preview delete should retain former footer object containment inside the deleted section");

            const auto restore_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--restore-object",
                    "--unique-id", "detail-footer-guid",
                    "--json"
                },
                temp_root);

            if (restore_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section preview restore stdout:\n"
                          << restore_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section preview restore stderr:\n"
                          << restore_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(restore_process.exit_code == 0,
                   "#1821: detail-footer section preview restore by stable selection should exit successfully");
            expect(!dbf_record_deleted(asset_path, 2U),
                   "#1821: detail-footer section preview restore should clear the deleted state");
            expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1821: detail-footer section preview restore should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                                "#1821: detail-footer label section preview restore should retain label identity");
            }
            expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1821: detail-footer section preview restore should preserve live preview availability");
            expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1821: detail-footer section preview restore should preserve live preview top bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#1821: detail-footer section preview restore should expand live preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#1821: detail-footer section preview restore should expand live preview heights");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#1821: detail-footer section preview restore should clear deleted preview availability");
            expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                            "#1821: detail-footer section preview restore should restore live section counts");
            expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                            "#1821: detail-footer section preview restore should clear deleted section counts");
            expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                            "#1821: detail-footer section preview restore should restore placed object counts");
            expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#1821: detail-footer section preview restore should clear unplaced object counts");
            expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1821: detail-footer section preview restore should preserve selected section availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1821: detail-footer section preview restore should preserve selection kind");
            expect_contains(restore_process.stdout_text, "\"dryRun\": false",
                            "#2241: detail-footer section preview restore JSON should expose committed state");
            expect_contains(restore_process.stdout_text, "\"mutatesAsset\": true",
                            "#2241: detail-footer section preview restore JSON should expose mutation state");
            expect_contains(restore_process.stdout_text, "\"undoAvailable\": true",
                            "#2241: detail-footer section preview restore JSON should expose undo availability");
            expect_contains(restore_process.stdout_text, "\"undoLabel\": \"Deleted state\"",
                            "#2241: detail-footer section preview restore JSON should expose the deleted-state undo label");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": false",
                    "\"sectionIndex\": 1",
                    "\"sectionCount\": 2",
                    "\"top\": 300",
                    "\"height\": 250",
                    "\"bottom\": 550"
                },
                "#1821: detail-footer section preview restore should refresh selected live-section geometry");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"objects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#1821: detail-footer section preview restore should restore footer object containment");
        };

    run_detail_header_delete_restore_preview_bounds(
        temp_root / "detail_header_section_delete_restore_preview_bounds.frx",
        "detail_header_section_delete_restore_preview_bounds.frx",
        "report");
    run_detail_header_delete_restore_preview_bounds(
        temp_root / "detail_header_section_delete_restore_preview_bounds.lbx",
        "detail_header_section_delete_restore_preview_bounds.lbx",
        "label");
    run_detail_footer_delete_restore_preview_bounds(
        temp_root / "detail_footer_section_delete_restore_preview_bounds.frx",
        "detail_footer_section_delete_restore_preview_bounds.frx",
        "report");
    run_detail_footer_delete_restore_preview_bounds(
        temp_root / "detail_footer_section_delete_restore_preview_bounds.lbx",
        "detail_footer_section_delete_restore_preview_bounds.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif
#endif

}  // namespace cf_test_studio_host_json
