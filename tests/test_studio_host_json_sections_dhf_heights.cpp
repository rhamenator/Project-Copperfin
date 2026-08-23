// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"


namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_ONLY_EXPRESSIONS)
void test_studio_host_json_updates_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_height_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_height_update =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "420",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section height update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section height update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1802: detail-header section height update by stable selection should exit successfully");
            const auto header_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "HEIGHT"
            });
            expect(header_height_property.ok && header_height_property.exists &&
                       header_height_property.value == "420",
                   "#1802: detail-header section height update should persist the HEIGHT field");
            expect_contains(update_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1802: detail-header section height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_process.stdout_text, "\"isLabel\": true",
                                "#1802: detail-header label section height update should retain label identity");
            }
            expect_contains(update_header_process.stdout_text, "\"sectionHeightTotal\": 670",
                            "#1802: detail-header section height update should refresh live section height totals");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1802: detail-header section height update should preserve deleted section height totals");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2276: detail-header section height update should preserve live preview availability");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2276: detail-header section height update should preserve live preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2276: detail-header section height update should preserve live preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2276: detail-header section height update should preserve live preview heights");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2276: detail-header section height update should preserve deleted preview availability");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2276: detail-header section height update should preserve deleted preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2276: detail-header section height update should preserve deleted preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1802: detail-header section height update should preserve selected section availability");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1802: detail-header section height update should preserve selection kind");
            expect_contains(update_header_process.stdout_text, "\"dryRun\": false",
                            "#2232: detail-header section height update JSON should expose committed state");
            expect_contains(update_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2232: detail-header section height update JSON should expose mutation state");
            expect_contains(update_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2232: detail-header section height update JSON should expose undo availability");
            expect_contains(update_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2232: detail-header section height update JSON should expose height undo labels");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 0",
                    "\"height\": 420",
                    "\"bottom\": 420"
                },
                "#1802: detail-header section height update should refresh selected-section geometry");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "280",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section height update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section height update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1802: detail-footer section height update by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.value == "280",
                   "#1802: detail-footer section height update should persist the HEIGHT field");
            expect_contains(update_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1802: detail-footer section height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_process.stdout_text, "\"isLabel\": true",
                                "#1802: detail-footer label section height update should retain label identity");
            }
            expect_contains(update_footer_process.stdout_text, "\"sectionHeightTotal\": 700",
                            "#1802: detail-footer section height update should refresh live section height totals");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1802: detail-footer section height update should preserve deleted section height totals");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2276: detail-footer section height update should preserve live preview availability");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2276: detail-footer section height update should preserve live preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsBottom\": 580",
                            "#2276: detail-footer section height update should refresh live preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsHeight\": 580",
                            "#2276: detail-footer section height update should refresh live preview heights");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2276: detail-footer section height update should preserve deleted preview availability");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2276: detail-footer section height update should preserve deleted preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2276: detail-footer section height update should preserve deleted preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1802: detail-footer section height update should preserve selected section availability");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1802: detail-footer section height update should preserve selection kind");
            expect_contains(update_footer_process.stdout_text, "\"dryRun\": false",
                            "#2232: detail-footer section height update JSON should expose committed state");
            expect_contains(update_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2232: detail-footer section height update JSON should expose mutation state");
            expect_contains(update_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2232: detail-footer section height update JSON should expose undo availability");
            expect_contains(update_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2232: detail-footer section height update JSON should expose height undo labels");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"objectCode\": 10",
                    "\"top\": 300",
                    "\"height\": 280",
                    "\"bottom\": 580"
                },
                "#1802: detail-footer section height update should refresh selected-section geometry");
        };

    run_detail_header_footer_section_height_update(
        temp_root / "detail_header_footer_section_height_stable.frx",
        "detail_header_footer_section_height_stable.frx",
        "report");
    run_detail_header_footer_section_height_update(
        temp_root / "detail_header_footer_section_height_stable.lbx",
        "detail_header_footer_section_height_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_height_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_height_clear =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section height clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section height clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1803: detail-header section height clear by stable selection should exit successfully");
            const auto header_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "HEIGHT"
            });
            expect(header_height_property.ok && header_height_property.exists &&
                       header_height_property.direct_field && header_height_property.value.empty(),
                   "#1803: detail-header section height clear should blank the HEIGHT field");
            expect_contains(clear_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1803: detail-header section height clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_header_process.stdout_text, "\"isLabel\": true",
                                "#1803: detail-header label section height clear should retain label identity");
            }
            expect_contains(clear_header_process.stdout_text, "\"sectionHeightTotal\": 250",
                            "#1803: detail-header section height clear should refresh live section height totals");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1803: detail-header section height clear should preserve deleted section height totals");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1822: detail-header section height clear should preserve live preview availability");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1822: detail-header section height clear should preserve live preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#1822: detail-header section height clear should preserve live preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#1822: detail-header section height clear should preserve live preview heights");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1822: detail-header section height clear should preserve deleted preview availability");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1822: detail-header section height clear should preserve deleted preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1822: detail-header section height clear should preserve deleted preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1803: detail-header section height clear should preserve selected section availability");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1803: detail-header section height clear should preserve selection kind");
            expect_contains(clear_header_process.stdout_text, "\"dryRun\": false",
                            "#2234: detail-header section height clear JSON should expose committed state");
            expect_contains(clear_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2234: detail-header section height clear JSON should expose mutation state");
            expect_contains(clear_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2234: detail-header section height clear JSON should expose undo availability");
            expect_contains(clear_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2234: detail-header section height clear JSON should expose height undo labels");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 0",
                    "\"height\": 0",
                    "\"bottom\": 0"
                },
                "#1803: detail-header section height clear should refresh selected-section geometry");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section height clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section height clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1803: detail-footer section height clear by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.direct_field && footer_height_property.value.empty(),
                   "#1803: detail-footer section height clear should blank the HEIGHT field");
            expect_contains(clear_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1803: detail-footer section height clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_footer_process.stdout_text, "\"isLabel\": true",
                                "#1803: detail-footer label section height clear should retain label identity");
            }
            expect_contains(clear_footer_process.stdout_text, "\"sectionHeightTotal\": 0",
                            "#1803: detail-footer section height clear should refresh live section height totals");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1803: detail-footer section height clear should preserve deleted section height totals");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1822: detail-footer section height clear should preserve live preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1822: detail-footer section height clear should preserve live preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsBottom\": 300",
                            "#1822: detail-footer section height clear should shrink live preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsHeight\": 300",
                            "#1822: detail-footer section height clear should shrink live preview heights");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1822: detail-footer section height clear should preserve deleted preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1822: detail-footer section height clear should preserve deleted preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1822: detail-footer section height clear should preserve deleted preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1803: detail-footer section height clear should preserve selected section availability");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1803: detail-footer section height clear should preserve selection kind");
            expect_contains(clear_footer_process.stdout_text, "\"dryRun\": false",
                            "#2234: detail-footer section height clear JSON should expose committed state");
            expect_contains(clear_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2234: detail-footer section height clear JSON should expose mutation state");
            expect_contains(clear_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2234: detail-footer section height clear JSON should expose undo availability");
            expect_contains(clear_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2234: detail-footer section height clear JSON should expose height undo labels");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"objectCode\": 10",
                    "\"top\": 300",
                    "\"height\": 0",
                    "\"bottom\": 300"
                },
                "#1803: detail-footer section height clear should refresh selected-section geometry");
        };

    run_detail_header_footer_section_height_clear(
        temp_root / "detail_header_footer_section_height_clear_stable.frx",
        "detail_header_footer_section_height_clear_stable.frx",
        "report");
    run_detail_header_footer_section_height_clear(
        temp_root / "detail_header_footer_section_height_clear_stable.lbx",
        "detail_header_footer_section_height_clear_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_height_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_height_update =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "360",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section height update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section height update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1804: deleted detail-header section height update by stable selection should exit successfully");
            const auto header_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "HEIGHT"
            });
            expect(header_height_property.ok && header_height_property.exists &&
                       header_height_property.value == "360",
                   "#1804: deleted detail-header section height update should persist the HEIGHT field");
            expect_contains(update_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1804: deleted detail-header section height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_process.stdout_text, "\"isLabel\": true",
                                "#1804: deleted detail-header label section height update should retain label identity");
            }
            expect_contains(update_header_process.stdout_text, "\"sectionCount\": 1",
                            "#1804: deleted detail-header section height update should preserve live section count");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1804: deleted detail-header section height update should preserve deleted section count");
            expect_contains(update_header_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1804: deleted detail-header section height update should preserve live section heights");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionHeightTotal\": 610",
                            "#1804: deleted detail-header section height update should refresh deleted section heights");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2236: deleted detail-header section height update should preserve live preview availability");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2236: deleted detail-header section height update should preserve live preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#2236: deleted detail-header section height update should preserve live preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2236: deleted detail-header section height update should preserve deleted preview availability");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#2236: deleted detail-header section height update should preserve deleted preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                            "#2236: deleted detail-header section height update should preserve deleted preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                            "#2236: deleted detail-header section height update should preserve deleted preview heights");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1804: deleted detail-header section height update should preserve selected section availability");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1804: deleted detail-header section height update should preserve selection kind");
            expect_contains(update_header_process.stdout_text, "\"dryRun\": false",
                            "#2236: deleted detail-header section height update JSON should expose committed state");
            expect_contains(update_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2236: deleted detail-header section height update JSON should expose mutation state");
            expect_contains(update_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2236: deleted detail-header section height update JSON should expose undo availability");
            expect_contains(update_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2236: deleted detail-header section height update JSON should expose height undo labels");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 500",
                    "\"height\": 360",
                    "\"bottom\": 860"
                },
                "#1804: deleted detail-header section height update should refresh selected-section geometry");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "290",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section height update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section height update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1804: deleted detail-footer section height update by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.value == "290",
                   "#1804: deleted detail-footer section height update should persist the HEIGHT field");
            expect_contains(update_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1804: deleted detail-footer section height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_process.stdout_text, "\"isLabel\": true",
                                "#1804: deleted detail-footer label section height update should retain label identity");
            }
            expect_contains(update_footer_process.stdout_text, "\"sectionCount\": 1",
                            "#1804: deleted detail-footer section height update should preserve live section count");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1804: deleted detail-footer section height update should preserve deleted section count");
            expect_contains(update_footer_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1804: deleted detail-footer section height update should preserve live section heights");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 650",
                            "#1804: deleted detail-footer section height update should refresh deleted section heights");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2236: deleted detail-footer section height update should preserve live preview availability");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2236: deleted detail-footer section height update should preserve live preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#2236: deleted detail-footer section height update should preserve live preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2236: deleted detail-footer section height update should preserve deleted preview availability");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#2236: deleted detail-footer section height update should preserve deleted preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1090",
                            "#2236: deleted detail-footer section height update should refresh deleted preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 590",
                            "#2236: deleted detail-footer section height update should refresh deleted preview heights");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1804: deleted detail-footer section height update should preserve selected section availability");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1804: deleted detail-footer section height update should preserve selection kind");
            expect_contains(update_footer_process.stdout_text, "\"dryRun\": false",
                            "#2236: deleted detail-footer section height update JSON should expose committed state");
            expect_contains(update_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2236: deleted detail-footer section height update JSON should expose mutation state");
            expect_contains(update_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2236: deleted detail-footer section height update JSON should expose undo availability");
            expect_contains(update_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2236: deleted detail-footer section height update JSON should expose height undo labels");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 10",
                    "\"top\": 800",
                    "\"height\": 290",
                    "\"bottom\": 1090"
                },
                "#1804: deleted detail-footer section height update should refresh selected-section geometry");
        };

    run_deleted_detail_header_footer_section_height_update(
        temp_root / "deleted_detail_header_footer_section_height_stable.frx",
        "deleted_detail_header_footer_section_height_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_height_update(
        temp_root / "deleted_detail_header_footer_section_height_stable.lbx",
        "deleted_detail_header_footer_section_height_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_height_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_height_clear =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section height clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section height clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1805: deleted detail-header section height clear by stable selection should exit successfully");
            const auto header_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "HEIGHT"
            });
            expect(header_height_property.ok && header_height_property.exists &&
                       header_height_property.direct_field && header_height_property.value.empty(),
                   "#1805: deleted detail-header section height clear should blank the HEIGHT field");
            expect_contains(clear_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1805: deleted detail-header section height clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_header_process.stdout_text, "\"isLabel\": true",
                                "#1805: deleted detail-header label section height clear should retain label identity");
            }
            expect_contains(clear_header_process.stdout_text, "\"sectionCount\": 1",
                            "#1805: deleted detail-header section height clear should preserve live section count");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1805: deleted detail-header section height clear should preserve deleted section count");
            expect_contains(clear_header_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1805: deleted detail-header section height clear should preserve live section heights");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionHeightTotal\": 250",
                            "#1805: deleted detail-header section height clear should refresh deleted section heights");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1822: deleted detail-header section height clear should preserve live preview availability");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1822: deleted detail-header section height clear should preserve live preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1822: deleted detail-header section height clear should preserve live preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1822: deleted detail-header section height clear should preserve deleted preview availability");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#1822: deleted detail-header section height clear should preserve deleted preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                            "#1822: deleted detail-header section height clear should preserve deleted preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                            "#1822: deleted detail-header section height clear should refresh deleted preview heights");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1805: deleted detail-header section height clear should preserve selected section availability");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1805: deleted detail-header section height clear should preserve selection kind");
            expect_contains(clear_header_process.stdout_text, "\"dryRun\": false",
                            "#2238: deleted detail-header section height clear JSON should expose committed state");
            expect_contains(clear_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2238: deleted detail-header section height clear JSON should expose mutation state");
            expect_contains(clear_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2238: deleted detail-header section height clear JSON should expose undo availability");
            expect_contains(clear_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2238: deleted detail-header section height clear JSON should expose height undo labels");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 500",
                    "\"height\": 0",
                    "\"bottom\": 500"
                },
                "#1805: deleted detail-header section height clear should refresh selected-section geometry");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section height clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section height clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1805: deleted detail-footer section height clear by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.direct_field && footer_height_property.value.empty(),
                   "#1805: deleted detail-footer section height clear should blank the HEIGHT field");
            expect_contains(clear_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1805: deleted detail-footer section height clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_footer_process.stdout_text, "\"isLabel\": true",
                                "#1805: deleted detail-footer label section height clear should retain label identity");
            }
            expect_contains(clear_footer_process.stdout_text, "\"sectionCount\": 1",
                            "#1805: deleted detail-footer section height clear should preserve live section count");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1805: deleted detail-footer section height clear should preserve deleted section count");
            expect_contains(clear_footer_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1805: deleted detail-footer section height clear should preserve live section heights");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 0",
                            "#1805: deleted detail-footer section height clear should refresh deleted section heights");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1822: deleted detail-footer section height clear should preserve live preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1822: deleted detail-footer section height clear should preserve live preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1822: deleted detail-footer section height clear should preserve live preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1822: deleted detail-footer section height clear should preserve deleted preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#1822: deleted detail-footer section height clear should preserve deleted preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 800",
                            "#1822: deleted detail-footer section height clear should shrink deleted preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                            "#1822: deleted detail-footer section height clear should shrink deleted preview heights");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1805: deleted detail-footer section height clear should preserve selected section availability");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1805: deleted detail-footer section height clear should preserve selection kind");
            expect_contains(clear_footer_process.stdout_text, "\"dryRun\": false",
                            "#2238: deleted detail-footer section height clear JSON should expose committed state");
            expect_contains(clear_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2238: deleted detail-footer section height clear JSON should expose mutation state");
            expect_contains(clear_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2238: deleted detail-footer section height clear JSON should expose undo availability");
            expect_contains(clear_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2238: deleted detail-footer section height clear JSON should expose height undo labels");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 10",
                    "\"top\": 800",
                    "\"height\": 0",
                    "\"bottom\": 800"
                },
                "#1805: deleted detail-footer section height clear should refresh selected-section geometry");
        };

    run_deleted_detail_header_footer_section_height_clear(
        temp_root / "deleted_detail_header_footer_section_height_clear_stable.frx",
        "deleted_detail_header_footer_section_height_clear_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_height_clear(
        temp_root / "deleted_detail_header_footer_section_height_clear_stable.lbx",
        "deleted_detail_header_footer_section_height_clear_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif
}  // namespace cf_test_studio_host_json
