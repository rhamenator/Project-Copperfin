// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_LAYOUT_ACTIONS_SKIP_STABLE)
void test_studio_host_json_aligns_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_align_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_align =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& title_prefix,
            const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(header_asset_path);
            write_synthetic_report_table_for_detail_header_footer_object_json(footer_asset_path);

            const auto align_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", header_asset_path.string(),
                    "--unique-id", "detail-header-label-guid",
                    "--align-object",
                    "--alignment-mode", "left",
                    "--anchor-unique-id", "detail-footer-field-guid",
                    "--align-target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (align_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object align stdout:\n"
                          << align_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object align stderr:\n"
                          << align_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(align_header_process.exit_code == 0,
                   "#1789: detail-header object align should exit successfully");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "HPOS") == "140" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "VPOS") == "50",
                   "#1789: detail-header object align should mutate HPOS and preserve VPOS");
            expect_contains(align_header_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_header." +
                                header_asset_path.extension().string().substr(1) + "\"",
                            "#1789: detail-header object align should return refreshed layout JSON");
            if (header_asset_path.extension() == ".lbx") {
                expect_contains(align_header_process.stdout_text, "\"isLabel\": true",
                                "#1789: detail-header label object align should retain label identity");
            }
            expect_contains(align_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1789: detail-header object align should preserve selected object availability");
            expect_contains(align_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1789: detail-header object align should preserve object selection kind");
            expect_contains(align_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1789: detail-header object align should preserve containing-section availability");
            expect_contains(align_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2295: detail-header object align should preserve live preview availability");
            expect_contains(align_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2295: detail-header object align should preserve live preview top bounds");
            expect_contains(align_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2295: detail-header object align should preserve live preview bottom bounds");
            expect_contains(align_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2295: detail-header object align should preserve live preview heights");
            expect_contains(align_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2295: detail-header object align should not fabricate deleted preview availability");
            expect_contains(align_header_process.stdout_text, "\"dryRun\": false",
                            "#2248: detail-header object align JSON should expose committed state");
            expect_contains(align_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2248: detail-header object align JSON should expose mutation state");
            expect_contains(align_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2248: detail-header object align JSON should expose undo availability");
            expect_contains(align_header_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                            "#2248: detail-header object align JSON should expose horizontal-position undo labels");
            expect_contains_in_order(
                align_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 140",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 840",
                    "\"height\": 120",
                    "\"bottom\": 170",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1789: detail-header object align should refresh selected-object section metadata");
            expect_contains_in_order(
                align_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1789: detail-header object align should preserve containing-section metadata");

            const auto align_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", footer_asset_path.string(),
                    "--unique-id", "detail-footer-field-guid",
                    "--align-object",
                    "--alignment-mode", "left",
                    "--anchor-unique-id", "detail-header-label-guid",
                    "--align-target-unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (align_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object align stdout:\n"
                          << align_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object align stderr:\n"
                          << align_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(align_footer_process.exit_code == 0,
                   "#1789: detail-footer object align should exit successfully");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "HPOS") == "100" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "VPOS") == "360",
                   "#1789: detail-footer object align should mutate HPOS and preserve VPOS");
            expect_contains(align_footer_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_footer." +
                                footer_asset_path.extension().string().substr(1) + "\"",
                            "#1789: detail-footer object align should return refreshed layout JSON");
            if (footer_asset_path.extension() == ".lbx") {
                expect_contains(align_footer_process.stdout_text, "\"isLabel\": true",
                                "#1789: detail-footer label object align should retain label identity");
            }
            expect_contains(align_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1789: detail-footer object align should preserve selected object availability");
            expect_contains(align_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1789: detail-footer object align should preserve object selection kind");
            expect_contains(align_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1789: detail-footer object align should preserve containing-section availability");
            expect_contains(align_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2295: detail-footer object align should preserve live preview availability");
            expect_contains(align_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2295: detail-footer object align should preserve live preview top bounds");
            expect_contains(align_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2295: detail-footer object align should preserve live preview bottom bounds");
            expect_contains(align_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2295: detail-footer object align should preserve live preview heights");
            expect_contains(align_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2295: detail-footer object align should not fabricate deleted preview availability");
            expect_contains(align_footer_process.stdout_text, "\"dryRun\": false",
                            "#2248: detail-footer object align JSON should expose committed state");
            expect_contains(align_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2248: detail-footer object align JSON should expose mutation state");
            expect_contains(align_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2248: detail-footer object align JSON should expose undo availability");
            expect_contains(align_footer_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                            "#2248: detail-footer object align JSON should expose horizontal-position undo labels");
            expect_contains_in_order(
                align_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 100",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1000",
                    "\"height\": 100",
                    "\"bottom\": 460",
                    "\"expression\": \"footer.total\""
                },
                "#1789: detail-footer object align should refresh selected-object section metadata");
            expect_contains_in_order(
                align_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1789: detail-footer object align should preserve containing-section metadata");
        };

    run_detail_header_footer_object_align(
        temp_root / "detail_header_footer_object_align_header.frx",
        temp_root / "detail_header_footer_object_align_footer.frx",
        "detail_header_footer_object_align",
        "report");
    run_detail_header_footer_object_align(
        temp_root / "detail_header_footer_object_align_header.lbx",
        temp_root / "detail_header_footer_object_align_footer.lbx",
        "detail_header_footer_object_align",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_aligns_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_align_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_align =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& title_prefix,
            const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(header_asset_path);
            write_synthetic_report_table_for_detail_header_footer_object_json(footer_asset_path);

            for (const auto& asset_path : {header_asset_path, footer_asset_path}) {
                for (const std::size_t record_index : {1U, 3U}) {
                    const auto delete_result =
                        copperfin::vfp::set_record_deleted_flag(asset_path.string(), record_index, true);
                    expect(delete_result.ok && dbf_record_deleted(asset_path, record_index),
                           "#1790: deleted detail header/footer object align fixture should mark object records deleted");
                }
            }

            const auto align_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", header_asset_path.string(),
                    "--unique-id", "detail-header-label-guid",
                    "--align-object",
                    "--alignment-mode", "left",
                    "--anchor-unique-id", "detail-footer-field-guid",
                    "--align-target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (align_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object align stdout:\n"
                          << align_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object align stderr:\n"
                          << align_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(align_header_process.exit_code == 0,
                   "#1790: deleted detail-header object align should exit successfully");
            expect(visual_object_deleted(header_asset_path, "detail-header-label-guid"),
                   "#1790: deleted detail-header object align should preserve deleted state");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "HPOS") == "140" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "VPOS") == "50",
                   "#1790: deleted detail-header object align should mutate HPOS and preserve VPOS");
            expect_contains(align_header_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_deleted_header." +
                                header_asset_path.extension().string().substr(1) + "\"",
                            "#1790: deleted detail-header object align should return refreshed layout JSON");
            if (header_asset_path.extension() == ".lbx") {
                expect_contains(align_header_process.stdout_text, "\"isLabel\": true",
                                "#1790: deleted detail-header label object align should retain label identity");
            }
            expect_contains(align_header_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1790: deleted detail-header object align should leave live object counts unchanged");
            expect_contains(align_header_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1790: deleted detail-header object align should preserve deleted object counts");
            expect_contains(align_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1790: deleted detail-header object align should preserve selected object availability");
            expect_contains(align_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1790: deleted detail-header object align should preserve object selection kind");
            expect_contains(align_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1790: deleted detail-header object align should preserve containing sections");
            expect_contains(align_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2296: deleted detail-header object align should preserve live preview availability");
            expect_contains(align_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2296: deleted detail-header object align should preserve live preview top bounds");
            expect_contains(align_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2296: deleted detail-header object align should preserve live preview bottom bounds");
            expect_contains(align_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2296: deleted detail-header object align should preserve live preview heights");
            expect_contains(align_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2296: deleted detail-header object align should expose deleted preview availability");
            expect_contains(align_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2296: deleted detail-header object align should preserve deleted preview top bounds");
            expect_contains(align_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2296: deleted detail-header object align should preserve deleted preview bottom bounds");
            expect_contains(align_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2296: deleted detail-header object align should preserve deleted preview heights");
            expect_contains(align_header_process.stdout_text, "\"dryRun\": false",
                            "#2249: deleted detail-header object align JSON should expose committed execution");
            expect_contains(align_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2249: deleted detail-header object align JSON should expose mutation state");
            expect_contains(align_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2249: deleted detail-header object align JSON should expose undo availability");
            expect_contains(align_header_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                            "#2249: deleted detail-header object align JSON should expose horizontal-position undo labels");
            expect_contains_in_order(
                align_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Header label\\\"\"",
                    "\"left\": 140",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 840",
                    "\"height\": 120",
                    "\"bottom\": 170"
                },
                "#1790: deleted detail-header object align should refresh selected deleted-object metadata");
            expect_contains_in_order(
                align_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1790: deleted detail-header object align should preserve containing-section metadata");

            const auto align_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", footer_asset_path.string(),
                    "--unique-id", "detail-footer-field-guid",
                    "--align-object",
                    "--alignment-mode", "left",
                    "--anchor-unique-id", "detail-header-label-guid",
                    "--align-target-unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (align_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object align stdout:\n"
                          << align_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object align stderr:\n"
                          << align_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(align_footer_process.exit_code == 0,
                   "#1790: deleted detail-footer object align should exit successfully");
            expect(visual_object_deleted(footer_asset_path, "detail-footer-field-guid"),
                   "#1790: deleted detail-footer object align should preserve deleted state");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "HPOS") == "100" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "VPOS") == "360",
                   "#1790: deleted detail-footer object align should mutate HPOS and preserve VPOS");
            expect_contains(align_footer_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_deleted_footer." +
                                footer_asset_path.extension().string().substr(1) + "\"",
                            "#1790: deleted detail-footer object align should return refreshed layout JSON");
            if (footer_asset_path.extension() == ".lbx") {
                expect_contains(align_footer_process.stdout_text, "\"isLabel\": true",
                                "#1790: deleted detail-footer label object align should retain label identity");
            }
            expect_contains(align_footer_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1790: deleted detail-footer object align should leave live object counts unchanged");
            expect_contains(align_footer_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1790: deleted detail-footer object align should preserve deleted object counts");
            expect_contains(align_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1790: deleted detail-footer object align should preserve selected object availability");
            expect_contains(align_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1790: deleted detail-footer object align should preserve object selection kind");
            expect_contains(align_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1790: deleted detail-footer object align should preserve containing sections");
            expect_contains(align_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2296: deleted detail-footer object align should preserve live preview availability");
            expect_contains(align_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2296: deleted detail-footer object align should preserve live preview top bounds");
            expect_contains(align_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2296: deleted detail-footer object align should preserve live preview bottom bounds");
            expect_contains(align_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2296: deleted detail-footer object align should preserve live preview heights");
            expect_contains(align_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2296: deleted detail-footer object align should expose deleted preview availability");
            expect_contains(align_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2296: deleted detail-footer object align should preserve deleted preview top bounds");
            expect_contains(align_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2296: deleted detail-footer object align should preserve deleted preview bottom bounds");
            expect_contains(align_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2296: deleted detail-footer object align should preserve deleted preview heights");
            expect_contains(align_footer_process.stdout_text, "\"dryRun\": false",
                            "#2249: deleted detail-footer object align JSON should expose committed execution");
            expect_contains(align_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2249: deleted detail-footer object align JSON should expose mutation state");
            expect_contains(align_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2249: deleted detail-footer object align JSON should expose undo availability");
            expect_contains(align_footer_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                            "#2249: deleted detail-footer object align JSON should expose horizontal-position undo labels");
            expect_contains_in_order(
                align_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"footer.total\"",
                    "\"left\": 100",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1000",
                    "\"height\": 100",
                    "\"bottom\": 460"
                },
                "#1790: deleted detail-footer object align should refresh selected deleted-object metadata");
            expect_contains_in_order(
                align_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1790: deleted detail-footer object align should preserve containing-section metadata");
        };

    run_deleted_detail_header_footer_object_align(
        temp_root / "detail_header_footer_object_align_deleted_header.frx",
        temp_root / "detail_header_footer_object_align_deleted_footer.frx",
        "detail_header_footer_object_align",
        "report");
    run_deleted_detail_header_footer_object_align(
        temp_root / "detail_header_footer_object_align_deleted_header.lbx",
        temp_root / "detail_header_footer_object_align_deleted_footer.lbx",
        "detail_header_footer_object_align",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_resizes_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_resize_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_resize =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& title_prefix,
            const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(header_asset_path);
            write_synthetic_report_table_for_detail_header_footer_object_json(footer_asset_path);

            const auto resize_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", header_asset_path.string(),
                    "--unique-id", "detail-header-label-guid",
                    "--resize-object",
                    "--resize-mode", "size",
                    "--anchor-unique-id", "detail-footer-field-guid",
                    "--resize-target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (resize_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object resize stdout:\n"
                          << resize_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object resize stderr:\n"
                          << resize_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(resize_header_process.exit_code == 0,
                   "#1791: detail-header object resize should exit successfully");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "WIDTH") == "900" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "HEIGHT") == "100",
                   "#1791: detail-header object resize should copy anchor size");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "HPOS") == "100" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "VPOS") == "50",
                   "#1791: detail-header object resize should preserve position");
            expect_contains(resize_header_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_header." +
                                header_asset_path.extension().string().substr(1) + "\"",
                            "#1791: detail-header object resize should return refreshed layout JSON");
            if (header_asset_path.extension() == ".lbx") {
                expect_contains(resize_header_process.stdout_text, "\"isLabel\": true",
                                "#1791: detail-header label object resize should retain label identity");
            }
            expect_contains(resize_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1791: detail-header object resize should preserve selected object availability");
            expect_contains(resize_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1791: detail-header object resize should preserve object selection kind");
            expect_contains(resize_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1791: detail-header object resize should preserve containing-section availability");
            expect_contains(resize_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2297: detail-header object resize should preserve live preview availability");
            expect_contains(resize_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2297: detail-header object resize should preserve live preview top bounds");
            expect_contains(resize_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2297: detail-header object resize should preserve live preview bottom bounds");
            expect_contains(resize_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2297: detail-header object resize should preserve live preview heights");
            expect_contains(resize_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2297: detail-header object resize should not fabricate deleted preview availability");
            expect_contains(resize_header_process.stdout_text, "\"dryRun\": false",
                            "#2250: detail-header object resize JSON should expose committed execution");
            expect_contains(resize_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2250: detail-header object resize JSON should expose mutation state");
            expect_contains(resize_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2250: detail-header object resize JSON should expose undo availability");
            expect_contains(resize_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2250: detail-header object resize JSON should expose resize undo labels");
            expect_contains_in_order(
                resize_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 150",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 900",
                    "\"right\": 1000",
                    "\"height\": 100",
                    "\"bottom\": 150",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1791: detail-header object resize should refresh selected-object section metadata");
            expect_contains_in_order(
                resize_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1791: detail-header object resize should preserve containing-section metadata");

            const auto resize_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", footer_asset_path.string(),
                    "--unique-id", "detail-footer-field-guid",
                    "--resize-object",
                    "--resize-mode", "size",
                    "--anchor-unique-id", "detail-header-label-guid",
                    "--resize-target-unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (resize_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object resize stdout:\n"
                          << resize_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object resize stderr:\n"
                          << resize_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(resize_footer_process.exit_code == 0,
                   "#1791: detail-footer object resize should exit successfully");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "WIDTH") == "700" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "HEIGHT") == "120",
                   "#1791: detail-footer object resize should copy anchor size");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "HPOS") == "140" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "VPOS") == "360",
                   "#1791: detail-footer object resize should preserve position");
            expect_contains(resize_footer_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_footer." +
                                footer_asset_path.extension().string().substr(1) + "\"",
                            "#1791: detail-footer object resize should return refreshed layout JSON");
            if (footer_asset_path.extension() == ".lbx") {
                expect_contains(resize_footer_process.stdout_text, "\"isLabel\": true",
                                "#1791: detail-footer label object resize should retain label identity");
            }
            expect_contains(resize_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1791: detail-footer object resize should preserve selected object availability");
            expect_contains(resize_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1791: detail-footer object resize should preserve object selection kind");
            expect_contains(resize_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1791: detail-footer object resize should preserve containing-section availability");
            expect_contains(resize_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2297: detail-footer object resize should preserve live preview availability");
            expect_contains(resize_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2297: detail-footer object resize should preserve live preview top bounds");
            expect_contains(resize_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2297: detail-footer object resize should preserve live preview bottom bounds");
            expect_contains(resize_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2297: detail-footer object resize should preserve live preview heights");
            expect_contains(resize_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2297: detail-footer object resize should not fabricate deleted preview availability");
            expect_contains(resize_footer_process.stdout_text, "\"dryRun\": false",
                            "#2250: detail-footer object resize JSON should expose committed execution");
            expect_contains(resize_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2250: detail-footer object resize JSON should expose mutation state");
            expect_contains(resize_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2250: detail-footer object resize JSON should expose undo availability");
            expect_contains(resize_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2250: detail-footer object resize JSON should expose resize undo labels");
            expect_contains_in_order(
                resize_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 180",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 700",
                    "\"right\": 840",
                    "\"height\": 120",
                    "\"bottom\": 480",
                    "\"expression\": \"footer.total\""
                },
                "#1791: detail-footer object resize should refresh selected-object section metadata");
            expect_contains_in_order(
                resize_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1791: detail-footer object resize should preserve containing-section metadata");
        };

    run_detail_header_footer_object_resize(
        temp_root / "detail_header_footer_object_resize_header.frx",
        temp_root / "detail_header_footer_object_resize_footer.frx",
        "detail_header_footer_object_resize",
        "report");
    run_detail_header_footer_object_resize(
        temp_root / "detail_header_footer_object_resize_header.lbx",
        temp_root / "detail_header_footer_object_resize_footer.lbx",
        "detail_header_footer_object_resize",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_resizes_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_resize_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_resize =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& title_prefix,
            const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(header_asset_path);
            write_synthetic_report_table_for_detail_header_footer_object_json(footer_asset_path);

            for (const auto& asset_path : {header_asset_path, footer_asset_path}) {
                for (const std::size_t record_index : {1U, 3U}) {
                    const auto delete_result =
                        copperfin::vfp::set_record_deleted_flag(asset_path.string(), record_index, true);
                    expect(delete_result.ok && dbf_record_deleted(asset_path, record_index),
                           "#1792: deleted detail header/footer object resize fixture should mark object records deleted");
                }
            }

            const auto resize_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", header_asset_path.string(),
                    "--unique-id", "detail-header-label-guid",
                    "--resize-object",
                    "--resize-mode", "size",
                    "--anchor-unique-id", "detail-footer-field-guid",
                    "--resize-target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (resize_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object resize stdout:\n"
                          << resize_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object resize stderr:\n"
                          << resize_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(resize_header_process.exit_code == 0,
                   "#1792: deleted detail-header object resize should exit successfully");
            expect(visual_object_deleted(header_asset_path, "detail-header-label-guid"),
                   "#1792: deleted detail-header object resize should preserve deleted state");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "WIDTH") == "900" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "HEIGHT") == "100",
                   "#1792: deleted detail-header object resize should copy anchor size");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "HPOS") == "100" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "VPOS") == "50",
                   "#1792: deleted detail-header object resize should preserve position");
            expect_contains(resize_header_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_deleted_header." +
                                header_asset_path.extension().string().substr(1) + "\"",
                            "#1792: deleted detail-header object resize should return refreshed layout JSON");
            if (header_asset_path.extension() == ".lbx") {
                expect_contains(resize_header_process.stdout_text, "\"isLabel\": true",
                                "#1792: deleted detail-header label object resize should retain label identity");
            }
            expect_contains(resize_header_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1792: deleted detail-header object resize should leave live object counts unchanged");
            expect_contains(resize_header_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1792: deleted detail-header object resize should preserve deleted object counts");
            expect_contains(resize_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1792: deleted detail-header object resize should preserve selected object availability");
            expect_contains(resize_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1792: deleted detail-header object resize should preserve object selection kind");
            expect_contains(resize_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1792: deleted detail-header object resize should preserve containing sections");
            expect_contains(resize_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2298: deleted detail-header object resize should preserve live preview availability");
            expect_contains(resize_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2298: deleted detail-header object resize should preserve live preview top bounds");
            expect_contains(resize_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2298: deleted detail-header object resize should preserve live preview bottom bounds");
            expect_contains(resize_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2298: deleted detail-header object resize should preserve live preview heights");
            expect_contains(resize_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2298: deleted detail-header object resize should expose deleted preview availability");
            expect_contains(resize_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2298: deleted detail-header object resize should preserve deleted preview top bounds");
            expect_contains(resize_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2298: deleted detail-header object resize should preserve deleted preview bottom bounds");
            expect_contains(resize_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2298: deleted detail-header object resize should preserve deleted preview heights");
            expect_contains(resize_header_process.stdout_text, "\"dryRun\": false",
                            "#2251: deleted detail-header object resize JSON should expose committed execution");
            expect_contains(resize_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2251: deleted detail-header object resize JSON should expose mutation state");
            expect_contains(resize_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2251: deleted detail-header object resize JSON should expose undo availability");
            expect_contains(resize_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2251: deleted detail-header object resize JSON should expose resize undo labels");
            expect_contains_in_order(
                resize_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 150",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Header label\\\"\"",
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 900",
                    "\"right\": 1000",
                    "\"height\": 100",
                    "\"bottom\": 150"
                },
                "#1792: deleted detail-header object resize should refresh selected deleted-object metadata");
            expect_contains_in_order(
                resize_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1792: deleted detail-header object resize should preserve containing-section metadata");

            const auto resize_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", footer_asset_path.string(),
                    "--unique-id", "detail-footer-field-guid",
                    "--resize-object",
                    "--resize-mode", "size",
                    "--anchor-unique-id", "detail-header-label-guid",
                    "--resize-target-unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (resize_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object resize stdout:\n"
                          << resize_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object resize stderr:\n"
                          << resize_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(resize_footer_process.exit_code == 0,
                   "#1792: deleted detail-footer object resize should exit successfully");
            expect(visual_object_deleted(footer_asset_path, "detail-footer-field-guid"),
                   "#1792: deleted detail-footer object resize should preserve deleted state");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "WIDTH") == "700" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "HEIGHT") == "120",
                   "#1792: deleted detail-footer object resize should copy anchor size");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "HPOS") == "140" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "VPOS") == "360",
                   "#1792: deleted detail-footer object resize should preserve position");
            expect_contains(resize_footer_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_deleted_footer." +
                                footer_asset_path.extension().string().substr(1) + "\"",
                            "#1792: deleted detail-footer object resize should return refreshed layout JSON");
            if (footer_asset_path.extension() == ".lbx") {
                expect_contains(resize_footer_process.stdout_text, "\"isLabel\": true",
                                "#1792: deleted detail-footer label object resize should retain label identity");
            }
            expect_contains(resize_footer_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1792: deleted detail-footer object resize should leave live object counts unchanged");
            expect_contains(resize_footer_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1792: deleted detail-footer object resize should preserve deleted object counts");
            expect_contains(resize_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1792: deleted detail-footer object resize should preserve selected object availability");
            expect_contains(resize_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1792: deleted detail-footer object resize should preserve object selection kind");
            expect_contains(resize_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1792: deleted detail-footer object resize should preserve containing sections");
            expect_contains(resize_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2298: deleted detail-footer object resize should preserve live preview availability");
            expect_contains(resize_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2298: deleted detail-footer object resize should preserve live preview top bounds");
            expect_contains(resize_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2298: deleted detail-footer object resize should preserve live preview bottom bounds");
            expect_contains(resize_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2298: deleted detail-footer object resize should preserve live preview heights");
            expect_contains(resize_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2298: deleted detail-footer object resize should expose deleted preview availability");
            expect_contains(resize_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2298: deleted detail-footer object resize should preserve deleted preview top bounds");
            expect_contains(resize_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 480",
                            "#2298: deleted detail-footer object resize should preserve deleted preview bottom bounds");
            expect_contains(resize_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 430",
                            "#2298: deleted detail-footer object resize should preserve deleted preview heights");
            expect_contains(resize_footer_process.stdout_text, "\"dryRun\": false",
                            "#2251: deleted detail-footer object resize JSON should expose committed execution");
            expect_contains(resize_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2251: deleted detail-footer object resize JSON should expose mutation state");
            expect_contains(resize_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2251: deleted detail-footer object resize JSON should expose undo availability");
            expect_contains(resize_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2251: deleted detail-footer object resize JSON should expose resize undo labels");
            expect_contains_in_order(
                resize_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 180",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"footer.total\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 700",
                    "\"right\": 840",
                    "\"height\": 120",
                    "\"bottom\": 480"
                },
                "#1792: deleted detail-footer object resize should refresh selected deleted-object metadata");
            expect_contains_in_order(
                resize_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1792: deleted detail-footer object resize should preserve containing-section metadata");
        };

    run_deleted_detail_header_footer_object_resize(
        temp_root / "detail_header_footer_object_resize_deleted_header.frx",
        temp_root / "detail_header_footer_object_resize_deleted_footer.frx",
        "detail_header_footer_object_resize",
        "report");
    run_deleted_detail_header_footer_object_resize(
        temp_root / "detail_header_footer_object_resize_deleted_header.lbx",
        temp_root / "detail_header_footer_object_resize_deleted_footer.lbx",
        "detail_header_footer_object_resize",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif
}  // namespace cf_test_studio_host_json
