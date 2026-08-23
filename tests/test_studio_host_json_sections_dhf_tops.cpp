// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_ONLY_EXPRESSIONS)
void test_studio_host_json_updates_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_top_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_top_update =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "VPOS",
                    "--property-value", "40",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section top update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section top update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1806: detail-header section top update by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.value == "40",
                   "#1806: detail-header section top update should persist the VPOS field");
            expect_contains(update_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1806: detail-header section top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_process.stdout_text, "\"isLabel\": true",
                                "#1806: detail-header label section top update should retain label identity");
            }
            expect_contains(update_header_process.stdout_text, "\"sectionHeightTotal\": 550",
                            "#1806: detail-header section top update should preserve live section height totals");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1806: detail-header section top update should preserve deleted section height totals");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2277: detail-header section top update should preserve live preview availability");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsTop\": 40",
                            "#2277: detail-header section top update should refresh live preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2277: detail-header section top update should preserve live preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsHeight\": 510",
                            "#2277: detail-header section top update should refresh live preview heights");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2277: detail-header section top update should preserve deleted preview availability");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2277: detail-header section top update should preserve deleted preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2277: detail-header section top update should preserve deleted preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                            "#2277: detail-header section top update should preserve deleted preview heights");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1806: detail-header section top update should preserve selected section availability");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1806: detail-header section top update should preserve selection kind");
            expect_contains(update_header_process.stdout_text, "\"dryRun\": false",
                            "#2233: detail-header section top update JSON should expose committed state");
            expect_contains(update_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2233: detail-header section top update JSON should expose mutation state");
            expect_contains(update_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2233: detail-header section top update JSON should expose undo availability");
            expect_contains(update_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2233: detail-header section top update JSON should expose top undo labels");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 40",
                    "\"height\": 300",
                    "\"bottom\": 340"
                },
                "#1806: detail-header section top update should refresh selected-section geometry");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "VPOS",
                    "--property-value", "360",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section top update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section top update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1806: detail-footer section top update by stable selection should exit successfully");
            const auto footer_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "VPOS"
            });
            expect(footer_top_property.ok && footer_top_property.exists &&
                       footer_top_property.value == "360",
                   "#1806: detail-footer section top update should persist the VPOS field");
            expect_contains(update_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1806: detail-footer section top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_process.stdout_text, "\"isLabel\": true",
                                "#1806: detail-footer label section top update should retain label identity");
            }
            expect_contains(update_footer_process.stdout_text, "\"sectionHeightTotal\": 550",
                            "#1806: detail-footer section top update should preserve live section height totals");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1806: detail-footer section top update should preserve deleted section height totals");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2277: detail-footer section top update should preserve live preview availability");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsTop\": 40",
                            "#2277: detail-footer section top update should preserve live preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsBottom\": 610",
                            "#2277: detail-footer section top update should refresh live preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsHeight\": 570",
                            "#2277: detail-footer section top update should refresh live preview heights");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2277: detail-footer section top update should preserve deleted preview availability");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2277: detail-footer section top update should preserve deleted preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2277: detail-footer section top update should preserve deleted preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                            "#2277: detail-footer section top update should preserve deleted preview heights");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1806: detail-footer section top update should preserve selected section availability");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1806: detail-footer section top update should preserve selection kind");
            expect_contains(update_footer_process.stdout_text, "\"dryRun\": false",
                            "#2233: detail-footer section top update JSON should expose committed state");
            expect_contains(update_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2233: detail-footer section top update JSON should expose mutation state");
            expect_contains(update_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2233: detail-footer section top update JSON should expose undo availability");
            expect_contains(update_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2233: detail-footer section top update JSON should expose top undo labels");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"objectCode\": 10",
                    "\"top\": 360",
                    "\"height\": 250",
                    "\"bottom\": 610"
                },
                "#1806: detail-footer section top update should refresh selected-section geometry");
        };

    run_detail_header_footer_section_top_update(
        temp_root / "detail_header_footer_section_top_stable.frx",
        "detail_header_footer_section_top_stable.frx",
        "report");
    run_detail_header_footer_section_top_update(
        temp_root / "detail_header_footer_section_top_stable.lbx",
        "detail_header_footer_section_top_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_top_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_top_clear =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "VPOS",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section top clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section top clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1807: detail-header section top clear by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.direct_field && header_top_property.value.empty(),
                   "#1807: detail-header section top clear should blank the VPOS field");
            expect_contains(clear_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1807: detail-header section top clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_header_process.stdout_text, "\"isLabel\": true",
                                "#1807: detail-header label section top clear should retain label identity");
            }
            expect_contains(clear_header_process.stdout_text, "\"sectionHeightTotal\": 550",
                            "#1807: detail-header section top clear should preserve live section height totals");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1807: detail-header section top clear should preserve deleted section height totals");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1823: detail-header section top clear should preserve live preview availability");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1823: detail-header section top clear should preserve live preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#1823: detail-header section top clear should preserve live preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#1823: detail-header section top clear should preserve live preview heights");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1823: detail-header section top clear should preserve deleted preview availability");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1823: detail-header section top clear should preserve deleted preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1823: detail-header section top clear should preserve deleted preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1807: detail-header section top clear should preserve selected section availability");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1807: detail-header section top clear should preserve selection kind");
            expect_contains(clear_header_process.stdout_text, "\"dryRun\": false",
                            "#2235: detail-header section top clear JSON should expose committed state");
            expect_contains(clear_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2235: detail-header section top clear JSON should expose mutation state");
            expect_contains(clear_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2235: detail-header section top clear JSON should expose undo availability");
            expect_contains(clear_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2235: detail-header section top clear JSON should expose top undo labels");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300"
                },
                "#1807: detail-header section top clear should refresh selected-section geometry");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "VPOS",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section top clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section top clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1807: detail-footer section top clear by stable selection should exit successfully");
            const auto footer_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "VPOS"
            });
            expect(footer_top_property.ok && footer_top_property.exists &&
                       footer_top_property.direct_field && footer_top_property.value.empty(),
                   "#1807: detail-footer section top clear should blank the VPOS field");
            expect_contains(clear_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1807: detail-footer section top clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_footer_process.stdout_text, "\"isLabel\": true",
                                "#1807: detail-footer label section top clear should retain label identity");
            }
            expect_contains(clear_footer_process.stdout_text, "\"sectionHeightTotal\": 550",
                            "#1807: detail-footer section top clear should preserve live section height totals");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1807: detail-footer section top clear should preserve deleted section height totals");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1823: detail-footer section top clear should preserve live preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1823: detail-footer section top clear should preserve live preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsBottom\": 300",
                            "#1823: detail-footer section top clear should shrink live preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsHeight\": 300",
                            "#1823: detail-footer section top clear should shrink live preview heights");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1823: detail-footer section top clear should preserve deleted preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1823: detail-footer section top clear should preserve deleted preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1823: detail-footer section top clear should preserve deleted preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1807: detail-footer section top clear should preserve selected section availability");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1807: detail-footer section top clear should preserve selection kind");
            expect_contains(clear_footer_process.stdout_text, "\"dryRun\": false",
                            "#2235: detail-footer section top clear JSON should expose committed state");
            expect_contains(clear_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2235: detail-footer section top clear JSON should expose mutation state");
            expect_contains(clear_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2235: detail-footer section top clear JSON should expose undo availability");
            expect_contains(clear_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2235: detail-footer section top clear JSON should expose top undo labels");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"objectCode\": 10",
                    "\"top\": 0",
                    "\"height\": 250",
                    "\"bottom\": 250"
                },
                "#1807: detail-footer section top clear should refresh selected-section geometry");
        };

    run_detail_header_footer_section_top_clear(
        temp_root / "detail_header_footer_section_top_clear_stable.frx",
        "detail_header_footer_section_top_clear_stable.frx",
        "report");
    run_detail_header_footer_section_top_clear(
        temp_root / "detail_header_footer_section_top_clear_stable.lbx",
        "detail_header_footer_section_top_clear_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_top_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_top_update =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "VPOS",
                    "--property-value", "620",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section top update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section top update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1808: deleted detail-header section top update by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.value == "620",
                   "#1808: deleted detail-header section top update should persist the VPOS field");
            expect_contains(update_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1808: deleted detail-header section top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_process.stdout_text, "\"isLabel\": true",
                                "#1808: deleted detail-header label section top update should retain label identity");
            }
            expect_contains(update_header_process.stdout_text, "\"sectionCount\": 1",
                            "#1808: deleted detail-header section top update should preserve live section count");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1808: deleted detail-header section top update should preserve deleted section count");
            expect_contains(update_header_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1808: deleted detail-header section top update should preserve live section heights");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                            "#1808: deleted detail-header section top update should preserve deleted section heights");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2237: deleted detail-header section top update should preserve live preview availability");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2237: deleted detail-header section top update should preserve live preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#2237: deleted detail-header section top update should preserve live preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2237: deleted detail-header section top update should preserve deleted preview availability");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 620",
                            "#2237: deleted detail-header section top update should refresh deleted preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                            "#2237: deleted detail-header section top update should preserve deleted preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 430",
                            "#2237: deleted detail-header section top update should refresh deleted preview heights");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1808: deleted detail-header section top update should preserve selected section availability");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1808: deleted detail-header section top update should preserve selection kind");
            expect_contains(update_header_process.stdout_text, "\"dryRun\": false",
                            "#2237: deleted detail-header section top update JSON should expose committed state");
            expect_contains(update_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2237: deleted detail-header section top update JSON should expose mutation state");
            expect_contains(update_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2237: deleted detail-header section top update JSON should expose undo availability");
            expect_contains(update_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2237: deleted detail-header section top update JSON should expose top undo labels");
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
                    "\"top\": 620",
                    "\"height\": 300",
                    "\"bottom\": 920"
                },
                "#1808: deleted detail-header section top update should refresh selected-section geometry");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "VPOS",
                    "--property-value", "940",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section top update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section top update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1808: deleted detail-footer section top update by stable selection should exit successfully");
            const auto footer_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "VPOS"
            });
            expect(footer_top_property.ok && footer_top_property.exists &&
                       footer_top_property.value == "940",
                   "#1808: deleted detail-footer section top update should persist the VPOS field");
            expect_contains(update_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1808: deleted detail-footer section top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_process.stdout_text, "\"isLabel\": true",
                                "#1808: deleted detail-footer label section top update should retain label identity");
            }
            expect_contains(update_footer_process.stdout_text, "\"sectionCount\": 1",
                            "#1808: deleted detail-footer section top update should preserve live section count");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1808: deleted detail-footer section top update should preserve deleted section count");
            expect_contains(update_footer_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1808: deleted detail-footer section top update should preserve live section heights");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                            "#1808: deleted detail-footer section top update should preserve deleted section heights");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2237: deleted detail-footer section top update should preserve live preview availability");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2237: deleted detail-footer section top update should preserve live preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#2237: deleted detail-footer section top update should preserve live preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2237: deleted detail-footer section top update should preserve deleted preview availability");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 620",
                            "#2237: deleted detail-footer section top update should preserve deleted preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1190",
                            "#2237: deleted detail-footer section top update should refresh deleted preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 570",
                            "#2237: deleted detail-footer section top update should refresh deleted preview heights");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1808: deleted detail-footer section top update should preserve selected section availability");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1808: deleted detail-footer section top update should preserve selection kind");
            expect_contains(update_footer_process.stdout_text, "\"dryRun\": false",
                            "#2237: deleted detail-footer section top update JSON should expose committed state");
            expect_contains(update_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2237: deleted detail-footer section top update JSON should expose mutation state");
            expect_contains(update_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2237: deleted detail-footer section top update JSON should expose undo availability");
            expect_contains(update_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2237: deleted detail-footer section top update JSON should expose top undo labels");
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
                    "\"top\": 940",
                    "\"height\": 250",
                    "\"bottom\": 1190"
                },
                "#1808: deleted detail-footer section top update should refresh selected-section geometry");
        };

    run_deleted_detail_header_footer_section_top_update(
        temp_root / "deleted_detail_header_footer_section_top_stable.frx",
        "deleted_detail_header_footer_section_top_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_top_update(
        temp_root / "deleted_detail_header_footer_section_top_stable.lbx",
        "deleted_detail_header_footer_section_top_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_top_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_top_clear =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "VPOS",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section top clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section top clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1809: deleted detail-header section top clear by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.direct_field && header_top_property.value.empty(),
                   "#1809: deleted detail-header section top clear should blank the VPOS field");
            expect_contains(clear_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1809: deleted detail-header section top clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_header_process.stdout_text, "\"isLabel\": true",
                                "#1809: deleted detail-header label section top clear should retain label identity");
            }
            expect_contains(clear_header_process.stdout_text, "\"sectionCount\": 1",
                            "#1809: deleted detail-header section top clear should preserve live section count");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1809: deleted detail-header section top clear should preserve deleted section count");
            expect_contains(clear_header_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1809: deleted detail-header section top clear should preserve live section heights");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                            "#1809: deleted detail-header section top clear should preserve deleted section heights");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1823: deleted detail-header section top clear should preserve live preview availability");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1823: deleted detail-header section top clear should preserve live preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1823: deleted detail-header section top clear should preserve live preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1823: deleted detail-header section top clear should preserve deleted preview availability");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                            "#1823: deleted detail-header section top clear should refresh deleted preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                            "#1823: deleted detail-header section top clear should preserve deleted preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 1050",
                            "#1823: deleted detail-header section top clear should refresh deleted preview heights");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1809: deleted detail-header section top clear should preserve selected section availability");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1809: deleted detail-header section top clear should preserve selection kind");
            expect_contains(clear_header_process.stdout_text, "\"dryRun\": false",
                            "#2239: deleted detail-header section top clear JSON should expose committed state");
            expect_contains(clear_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2239: deleted detail-header section top clear JSON should expose mutation state");
            expect_contains(clear_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2239: deleted detail-header section top clear JSON should expose undo availability");
            expect_contains(clear_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2239: deleted detail-header section top clear JSON should expose top undo labels");
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
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300"
                },
                "#1809: deleted detail-header section top clear should refresh selected-section geometry");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "VPOS",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section top clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section top clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1809: deleted detail-footer section top clear by stable selection should exit successfully");
            const auto footer_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "VPOS"
            });
            expect(footer_top_property.ok && footer_top_property.exists &&
                       footer_top_property.direct_field && footer_top_property.value.empty(),
                   "#1809: deleted detail-footer section top clear should blank the VPOS field");
            expect_contains(clear_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1809: deleted detail-footer section top clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_footer_process.stdout_text, "\"isLabel\": true",
                                "#1809: deleted detail-footer label section top clear should retain label identity");
            }
            expect_contains(clear_footer_process.stdout_text, "\"sectionCount\": 1",
                            "#1809: deleted detail-footer section top clear should preserve live section count");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1809: deleted detail-footer section top clear should preserve deleted section count");
            expect_contains(clear_footer_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1809: deleted detail-footer section top clear should preserve live section heights");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                            "#1809: deleted detail-footer section top clear should preserve deleted section heights");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1823: deleted detail-footer section top clear should preserve live preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1823: deleted detail-footer section top clear should preserve live preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1823: deleted detail-footer section top clear should preserve live preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1823: deleted detail-footer section top clear should preserve deleted preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                            "#1823: deleted detail-footer section top clear should preserve deleted preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 300",
                            "#1823: deleted detail-footer section top clear should shrink deleted preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                            "#1823: deleted detail-footer section top clear should shrink deleted preview heights");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1809: deleted detail-footer section top clear should preserve selected section availability");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1809: deleted detail-footer section top clear should preserve selection kind");
            expect_contains(clear_footer_process.stdout_text, "\"dryRun\": false",
                            "#2239: deleted detail-footer section top clear JSON should expose committed state");
            expect_contains(clear_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2239: deleted detail-footer section top clear JSON should expose mutation state");
            expect_contains(clear_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2239: deleted detail-footer section top clear JSON should expose undo availability");
            expect_contains(clear_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2239: deleted detail-footer section top clear JSON should expose top undo labels");
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
                    "\"top\": 0",
                    "\"height\": 250",
                    "\"bottom\": 250"
                },
                "#1809: deleted detail-footer section top clear should refresh selected-section geometry");
        };

    run_deleted_detail_header_footer_section_top_clear(
        temp_root / "deleted_detail_header_footer_section_top_clear_stable.frx",
        "deleted_detail_header_footer_section_top_clear_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_top_clear(
        temp_root / "deleted_detail_header_footer_section_top_clear_stable.lbx",
        "deleted_detail_header_footer_section_top_clear_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif
}  // namespace cf_test_studio_host_json
