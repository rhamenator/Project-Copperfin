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

void test_studio_host_json_snaps_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_snap_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_snap =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& title_prefix,
            const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(header_asset_path);
            write_synthetic_report_table_for_detail_header_footer_object_json(footer_asset_path);

            const auto snap_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", header_asset_path.string(),
                    "--unique-id", "detail-header-label-guid",
                    "--snap-object",
                    "--snap-mode", "both",
                    "--grid-width", "80",
                    "--grid-height", "70",
                    "--snap-target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (snap_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object snap stdout:\n"
                          << snap_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object snap stderr:\n"
                          << snap_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(snap_header_process.exit_code == 0,
                   "#1793: detail-header object snap should exit successfully");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "HPOS") == "80" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "VPOS") == "70",
                   "#1793: detail-header object snap should round HPOS and VPOS");
            expect_contains(snap_header_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_header." +
                                header_asset_path.extension().string().substr(1) + "\"",
                            "#1793: detail-header object snap should return refreshed layout JSON");
            if (header_asset_path.extension() == ".lbx") {
                expect_contains(snap_header_process.stdout_text, "\"isLabel\": true",
                                "#1793: detail-header label object snap should retain label identity");
            }
            expect_contains(snap_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1793: detail-header object snap should preserve selected object availability");
            expect_contains(snap_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1793: detail-header object snap should preserve object selection kind");
            expect_contains(snap_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1793: detail-header object snap should preserve containing-section availability");
            expect_contains(snap_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2299: detail-header object snap should preserve live preview availability");
            expect_contains(snap_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2299: detail-header object snap should preserve live preview top bounds");
            expect_contains(snap_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2299: detail-header object snap should preserve live preview bottom bounds");
            expect_contains(snap_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2299: detail-header object snap should preserve live preview heights");
            expect_contains(snap_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2299: detail-header object snap should not fabricate deleted preview availability");
            expect_contains(snap_header_process.stdout_text, "\"dryRun\": false",
                            "#2252: detail-header object snap JSON should expose committed execution");
            expect_contains(snap_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2252: detail-header object snap JSON should expose mutation state");
            expect_contains(snap_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2252: detail-header object snap JSON should expose undo availability");
            expect_contains(snap_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2252: detail-header object snap JSON should expose snap undo labels");
            expect_contains_in_order(
                snap_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 70",
                    "\"sectionRelativeBottom\": 190",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 80",
                    "\"top\": 70",
                    "\"width\": 700",
                    "\"right\": 780",
                    "\"height\": 120",
                    "\"bottom\": 190",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1793: detail-header object snap should refresh selected-object section metadata");
            expect_contains_in_order(
                snap_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1793: detail-header object snap should preserve containing-section metadata");

            const auto snap_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", footer_asset_path.string(),
                    "--unique-id", "detail-footer-field-guid",
                    "--snap-object",
                    "--snap-mode", "both",
                    "--grid-width", "80",
                    "--grid-height", "70",
                    "--snap-target-unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (snap_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object snap stdout:\n"
                          << snap_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object snap stderr:\n"
                          << snap_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(snap_footer_process.exit_code == 0,
                   "#1793: detail-footer object snap should exit successfully");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "HPOS") == "160" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "VPOS") == "350",
                   "#1793: detail-footer object snap should round HPOS and VPOS");
            expect_contains(snap_footer_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_footer." +
                                footer_asset_path.extension().string().substr(1) + "\"",
                            "#1793: detail-footer object snap should return refreshed layout JSON");
            if (footer_asset_path.extension() == ".lbx") {
                expect_contains(snap_footer_process.stdout_text, "\"isLabel\": true",
                                "#1793: detail-footer label object snap should retain label identity");
            }
            expect_contains(snap_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1793: detail-footer object snap should preserve selected object availability");
            expect_contains(snap_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1793: detail-footer object snap should preserve object selection kind");
            expect_contains(snap_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1793: detail-footer object snap should preserve containing-section availability");
            expect_contains(snap_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2299: detail-footer object snap should preserve live preview availability");
            expect_contains(snap_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2299: detail-footer object snap should preserve live preview top bounds");
            expect_contains(snap_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2299: detail-footer object snap should preserve live preview bottom bounds");
            expect_contains(snap_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2299: detail-footer object snap should preserve live preview heights");
            expect_contains(snap_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2299: detail-footer object snap should not fabricate deleted preview availability");
            expect_contains(snap_footer_process.stdout_text, "\"dryRun\": false",
                            "#2252: detail-footer object snap JSON should expose committed execution");
            expect_contains(snap_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2252: detail-footer object snap JSON should expose mutation state");
            expect_contains(snap_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2252: detail-footer object snap JSON should expose undo availability");
            expect_contains(snap_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2252: detail-footer object snap JSON should expose snap undo labels");
            expect_contains_in_order(
                snap_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 150",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 160",
                    "\"top\": 350",
                    "\"width\": 900",
                    "\"right\": 1060",
                    "\"height\": 100",
                    "\"bottom\": 450",
                    "\"expression\": \"footer.total\""
                },
                "#1793: detail-footer object snap should refresh selected-object section metadata");
            expect_contains_in_order(
                snap_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1793: detail-footer object snap should preserve containing-section metadata");
        };

    run_detail_header_footer_object_snap(
        temp_root / "detail_header_footer_object_snap_header.frx",
        temp_root / "detail_header_footer_object_snap_footer.frx",
        "detail_header_footer_object_snap",
        "report");
    run_detail_header_footer_object_snap(
        temp_root / "detail_header_footer_object_snap_header.lbx",
        temp_root / "detail_header_footer_object_snap_footer.lbx",
        "detail_header_footer_object_snap",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_snaps_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_snap_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_snap =
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
                           "#1794: deleted detail header/footer object snap fixture should mark object records deleted");
                }
            }

            const auto snap_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", header_asset_path.string(),
                    "--unique-id", "detail-header-label-guid",
                    "--snap-object",
                    "--snap-mode", "both",
                    "--grid-width", "80",
                    "--grid-height", "70",
                    "--snap-target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (snap_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object snap stdout:\n"
                          << snap_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object snap stderr:\n"
                          << snap_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(snap_header_process.exit_code == 0,
                   "#1794: deleted detail-header object snap should exit successfully");
            expect(visual_object_deleted(header_asset_path, "detail-header-label-guid"),
                   "#1794: deleted detail-header object snap should preserve deleted state");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "HPOS") == "80" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "VPOS") == "70",
                   "#1794: deleted detail-header object snap should round HPOS and VPOS");
            expect_contains(snap_header_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_deleted_header." +
                                header_asset_path.extension().string().substr(1) + "\"",
                            "#1794: deleted detail-header object snap should return refreshed layout JSON");
            if (header_asset_path.extension() == ".lbx") {
                expect_contains(snap_header_process.stdout_text, "\"isLabel\": true",
                                "#1794: deleted detail-header label object snap should retain label identity");
            }
            expect_contains(snap_header_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1794: deleted detail-header object snap should leave live object counts unchanged");
            expect_contains(snap_header_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1794: deleted detail-header object snap should preserve deleted object counts");
            expect_contains(snap_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1794: deleted detail-header object snap should preserve selected object availability");
            expect_contains(snap_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1794: deleted detail-header object snap should preserve object selection kind");
            expect_contains(snap_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1794: deleted detail-header object snap should preserve containing sections");
            expect_contains(snap_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2300: deleted detail-header object snap should preserve live preview availability");
            expect_contains(snap_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2300: deleted detail-header object snap should preserve live preview top bounds");
            expect_contains(snap_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2300: deleted detail-header object snap should preserve live preview bottom bounds");
            expect_contains(snap_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2300: deleted detail-header object snap should preserve live preview heights");
            expect_contains(snap_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2300: deleted detail-header object snap should expose deleted preview availability");
            expect_contains(snap_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 70",
                            "#2300: deleted detail-header object snap should refresh deleted preview top bounds");
            expect_contains(snap_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2300: deleted detail-header object snap should preserve deleted preview bottom bounds");
            expect_contains(snap_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 390",
                            "#2300: deleted detail-header object snap should refresh deleted preview heights");
            expect_contains(snap_header_process.stdout_text, "\"dryRun\": false",
                            "#2253: deleted detail-header object snap JSON should expose committed execution");
            expect_contains(snap_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2253: deleted detail-header object snap JSON should expose mutation state");
            expect_contains(snap_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2253: deleted detail-header object snap JSON should expose undo availability");
            expect_contains(snap_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2253: deleted detail-header object snap JSON should expose snap undo labels");
            expect_contains_in_order(
                snap_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 70",
                    "\"sectionRelativeBottom\": 190",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Header label\\\"\"",
                    "\"left\": 80",
                    "\"top\": 70",
                    "\"width\": 700",
                    "\"right\": 780",
                    "\"height\": 120",
                    "\"bottom\": 190"
                },
                "#1794: deleted detail-header object snap should refresh selected deleted-object metadata");
            expect_contains_in_order(
                snap_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1794: deleted detail-header object snap should preserve containing-section metadata");

            const auto snap_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", footer_asset_path.string(),
                    "--unique-id", "detail-footer-field-guid",
                    "--snap-object",
                    "--snap-mode", "both",
                    "--grid-width", "80",
                    "--grid-height", "70",
                    "--snap-target-unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (snap_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object snap stdout:\n"
                          << snap_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object snap stderr:\n"
                          << snap_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(snap_footer_process.exit_code == 0,
                   "#1794: deleted detail-footer object snap should exit successfully");
            expect(visual_object_deleted(footer_asset_path, "detail-footer-field-guid"),
                   "#1794: deleted detail-footer object snap should preserve deleted state");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "HPOS") == "160" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "VPOS") == "350",
                   "#1794: deleted detail-footer object snap should round HPOS and VPOS");
            expect_contains(snap_footer_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_deleted_footer." +
                                footer_asset_path.extension().string().substr(1) + "\"",
                            "#1794: deleted detail-footer object snap should return refreshed layout JSON");
            if (footer_asset_path.extension() == ".lbx") {
                expect_contains(snap_footer_process.stdout_text, "\"isLabel\": true",
                                "#1794: deleted detail-footer label object snap should retain label identity");
            }
            expect_contains(snap_footer_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1794: deleted detail-footer object snap should leave live object counts unchanged");
            expect_contains(snap_footer_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1794: deleted detail-footer object snap should preserve deleted object counts");
            expect_contains(snap_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1794: deleted detail-footer object snap should preserve selected object availability");
            expect_contains(snap_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1794: deleted detail-footer object snap should preserve object selection kind");
            expect_contains(snap_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1794: deleted detail-footer object snap should preserve containing sections");
            expect_contains(snap_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2300: deleted detail-footer object snap should preserve live preview availability");
            expect_contains(snap_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2300: deleted detail-footer object snap should preserve live preview top bounds");
            expect_contains(snap_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2300: deleted detail-footer object snap should preserve live preview bottom bounds");
            expect_contains(snap_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2300: deleted detail-footer object snap should preserve live preview heights");
            expect_contains(snap_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2300: deleted detail-footer object snap should expose deleted preview availability");
            expect_contains(snap_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2300: deleted detail-footer object snap should preserve deleted preview top bounds");
            expect_contains(snap_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 450",
                            "#2300: deleted detail-footer object snap should refresh deleted preview bottom bounds");
            expect_contains(snap_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 400",
                            "#2300: deleted detail-footer object snap should refresh deleted preview heights");
            expect_contains(snap_footer_process.stdout_text, "\"dryRun\": false",
                            "#2253: deleted detail-footer object snap JSON should expose committed execution");
            expect_contains(snap_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2253: deleted detail-footer object snap JSON should expose mutation state");
            expect_contains(snap_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2253: deleted detail-footer object snap JSON should expose undo availability");
            expect_contains(snap_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2253: deleted detail-footer object snap JSON should expose snap undo labels");
            expect_contains_in_order(
                snap_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 150",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"footer.total\"",
                    "\"left\": 160",
                    "\"top\": 350",
                    "\"width\": 900",
                    "\"right\": 1060",
                    "\"height\": 100",
                    "\"bottom\": 450"
                },
                "#1794: deleted detail-footer object snap should refresh selected deleted-object metadata");
            expect_contains_in_order(
                snap_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1794: deleted detail-footer object snap should preserve containing-section metadata");
        };

    run_deleted_detail_header_footer_object_snap(
        temp_root / "detail_header_footer_object_snap_deleted_header.frx",
        temp_root / "detail_header_footer_object_snap_deleted_footer.frx",
        "detail_header_footer_object_snap",
        "report");
    run_deleted_detail_header_footer_object_snap(
        temp_root / "detail_header_footer_object_snap_deleted_header.lbx",
        temp_root / "detail_header_footer_object_snap_deleted_footer.lbx",
        "detail_header_footer_object_snap",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_nudges_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_nudge_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_nudge =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& title_prefix,
            const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(header_asset_path);
            write_synthetic_report_table_for_detail_header_footer_object_json(footer_asset_path);

            const auto nudge_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", header_asset_path.string(),
                    "--unique-id", "detail-header-label-guid",
                    "--nudge-object",
                    "--nudge-mode", "both",
                    "--delta-hpos", "25",
                    "--delta-vpos", "-20",
                    "--nudge-target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (nudge_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object nudge stdout:\n"
                          << nudge_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object nudge stderr:\n"
                          << nudge_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(nudge_header_process.exit_code == 0,
                   "#1795: detail-header object nudge should exit successfully");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "HPOS") == "125" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "VPOS") == "30",
                   "#1795: detail-header object nudge should mutate HPOS and VPOS");
            expect_contains(nudge_header_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_header." +
                                header_asset_path.extension().string().substr(1) + "\"",
                            "#1795: detail-header object nudge should return refreshed layout JSON");
            if (header_asset_path.extension() == ".lbx") {
                expect_contains(nudge_header_process.stdout_text, "\"isLabel\": true",
                                "#1795: detail-header label object nudge should retain label identity");
            }
            expect_contains(nudge_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1795: detail-header object nudge should preserve selected object availability");
            expect_contains(nudge_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1795: detail-header object nudge should preserve object selection kind");
            expect_contains(nudge_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1795: detail-header object nudge should preserve containing-section availability");
            expect_contains(nudge_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2301: detail-header object nudge should preserve live preview availability");
            expect_contains(nudge_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2301: detail-header object nudge should preserve live preview top bounds");
            expect_contains(nudge_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2301: detail-header object nudge should preserve live preview bottom bounds");
            expect_contains(nudge_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2301: detail-header object nudge should preserve live preview heights");
            expect_contains(nudge_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2301: detail-header object nudge should not fabricate deleted preview availability");
            expect_contains(nudge_header_process.stdout_text, "\"dryRun\": false",
                            "#2254: detail-header object nudge JSON should expose committed execution");
            expect_contains(nudge_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2254: detail-header object nudge JSON should expose mutation state");
            expect_contains(nudge_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2254: detail-header object nudge JSON should expose undo availability");
            expect_contains(nudge_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2254: detail-header object nudge JSON should expose nudge undo labels");
            expect_contains_in_order(
                nudge_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 30",
                    "\"sectionRelativeBottom\": 150",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 125",
                    "\"top\": 30",
                    "\"width\": 700",
                    "\"right\": 825",
                    "\"height\": 120",
                    "\"bottom\": 150",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1795: detail-header object nudge should refresh selected-object section metadata");
            expect_contains_in_order(
                nudge_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1795: detail-header object nudge should preserve containing-section metadata");

            const auto nudge_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", footer_asset_path.string(),
                    "--unique-id", "detail-footer-field-guid",
                    "--nudge-object",
                    "--nudge-mode", "both",
                    "--delta-hpos", "25",
                    "--delta-vpos", "-20",
                    "--nudge-target-unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (nudge_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object nudge stdout:\n"
                          << nudge_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object nudge stderr:\n"
                          << nudge_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(nudge_footer_process.exit_code == 0,
                   "#1795: detail-footer object nudge should exit successfully");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "HPOS") == "165" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "VPOS") == "340",
                   "#1795: detail-footer object nudge should mutate HPOS and VPOS");
            expect_contains(nudge_footer_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_footer." +
                                footer_asset_path.extension().string().substr(1) + "\"",
                            "#1795: detail-footer object nudge should return refreshed layout JSON");
            if (footer_asset_path.extension() == ".lbx") {
                expect_contains(nudge_footer_process.stdout_text, "\"isLabel\": true",
                                "#1795: detail-footer label object nudge should retain label identity");
            }
            expect_contains(nudge_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1795: detail-footer object nudge should preserve selected object availability");
            expect_contains(nudge_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1795: detail-footer object nudge should preserve object selection kind");
            expect_contains(nudge_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1795: detail-footer object nudge should preserve containing-section availability");
            expect_contains(nudge_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2301: detail-footer object nudge should preserve live preview availability");
            expect_contains(nudge_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2301: detail-footer object nudge should preserve live preview top bounds");
            expect_contains(nudge_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2301: detail-footer object nudge should preserve live preview bottom bounds");
            expect_contains(nudge_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2301: detail-footer object nudge should preserve live preview heights");
            expect_contains(nudge_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2301: detail-footer object nudge should not fabricate deleted preview availability");
            expect_contains(nudge_footer_process.stdout_text, "\"dryRun\": false",
                            "#2254: detail-footer object nudge JSON should expose committed execution");
            expect_contains(nudge_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2254: detail-footer object nudge JSON should expose mutation state");
            expect_contains(nudge_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2254: detail-footer object nudge JSON should expose undo availability");
            expect_contains(nudge_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2254: detail-footer object nudge JSON should expose nudge undo labels");
            expect_contains_in_order(
                nudge_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 40",
                    "\"sectionRelativeBottom\": 140",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 165",
                    "\"top\": 340",
                    "\"width\": 900",
                    "\"right\": 1065",
                    "\"height\": 100",
                    "\"bottom\": 440",
                    "\"expression\": \"footer.total\""
                },
                "#1795: detail-footer object nudge should refresh selected-object section metadata");
            expect_contains_in_order(
                nudge_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1795: detail-footer object nudge should preserve containing-section metadata");
        };

    run_detail_header_footer_object_nudge(
        temp_root / "detail_header_footer_object_nudge_header.frx",
        temp_root / "detail_header_footer_object_nudge_footer.frx",
        "detail_header_footer_object_nudge",
        "report");
    run_detail_header_footer_object_nudge(
        temp_root / "detail_header_footer_object_nudge_header.lbx",
        temp_root / "detail_header_footer_object_nudge_footer.lbx",
        "detail_header_footer_object_nudge",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_nudges_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_nudge_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_nudge =
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
                           "#1796: deleted detail header/footer object nudge fixture should mark object records deleted");
                }
            }

            const auto nudge_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", header_asset_path.string(),
                    "--unique-id", "detail-header-label-guid",
                    "--nudge-object",
                    "--nudge-mode", "both",
                    "--delta-hpos", "25",
                    "--delta-vpos", "-20",
                    "--nudge-target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (nudge_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object nudge stdout:\n"
                          << nudge_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object nudge stderr:\n"
                          << nudge_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(nudge_header_process.exit_code == 0,
                   "#1796: deleted detail-header object nudge should exit successfully");
            expect(visual_object_deleted(header_asset_path, "detail-header-label-guid"),
                   "#1796: deleted detail-header object nudge should preserve deleted state");
            expect(visual_object_property(header_asset_path, "detail-header-label-guid", "HPOS") == "125" &&
                       visual_object_property(header_asset_path, "detail-header-label-guid", "VPOS") == "30",
                   "#1796: deleted detail-header object nudge should mutate HPOS and VPOS");
            expect_contains(nudge_header_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_deleted_header." +
                                header_asset_path.extension().string().substr(1) + "\"",
                            "#1796: deleted detail-header object nudge should return refreshed layout JSON");
            if (header_asset_path.extension() == ".lbx") {
                expect_contains(nudge_header_process.stdout_text, "\"isLabel\": true",
                                "#1796: deleted detail-header label object nudge should retain label identity");
            }
            expect_contains(nudge_header_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1796: deleted detail-header object nudge should leave live object counts unchanged");
            expect_contains(nudge_header_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1796: deleted detail-header object nudge should preserve deleted object counts");
            expect_contains(nudge_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1796: deleted detail-header object nudge should preserve selected object availability");
            expect_contains(nudge_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1796: deleted detail-header object nudge should preserve object selection kind");
            expect_contains(nudge_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1796: deleted detail-header object nudge should preserve containing sections");
            expect_contains(nudge_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2302: deleted detail-header object nudge should preserve live preview availability");
            expect_contains(nudge_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2302: deleted detail-header object nudge should preserve live preview top bounds");
            expect_contains(nudge_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2302: deleted detail-header object nudge should preserve live preview bottom bounds");
            expect_contains(nudge_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2302: deleted detail-header object nudge should preserve live preview heights");
            expect_contains(nudge_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2302: deleted detail-header object nudge should expose deleted preview availability");
            expect_contains(nudge_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 30",
                            "#2302: deleted detail-header object nudge should refresh deleted preview top bounds");
            expect_contains(nudge_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2302: deleted detail-header object nudge should preserve deleted preview bottom bounds");
            expect_contains(nudge_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 430",
                            "#2302: deleted detail-header object nudge should refresh deleted preview heights");
            expect_contains(nudge_header_process.stdout_text, "\"dryRun\": false",
                            "#2255: deleted detail-header object nudge JSON should expose committed execution");
            expect_contains(nudge_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2255: deleted detail-header object nudge JSON should expose mutation state");
            expect_contains(nudge_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2255: deleted detail-header object nudge JSON should expose undo availability");
            expect_contains(nudge_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2255: deleted detail-header object nudge JSON should expose nudge undo labels");
            expect_contains_in_order(
                nudge_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 30",
                    "\"sectionRelativeBottom\": 150",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Header label\\\"\"",
                    "\"left\": 125",
                    "\"top\": 30",
                    "\"width\": 700",
                    "\"right\": 825",
                    "\"height\": 120",
                    "\"bottom\": 150"
                },
                "#1796: deleted detail-header object nudge should refresh selected deleted-object metadata");
            expect_contains_in_order(
                nudge_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1796: deleted detail-header object nudge should preserve containing-section metadata");

            const auto nudge_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", footer_asset_path.string(),
                    "--unique-id", "detail-footer-field-guid",
                    "--nudge-object",
                    "--nudge-mode", "both",
                    "--delta-hpos", "25",
                    "--delta-vpos", "-20",
                    "--nudge-target-unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (nudge_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object nudge stdout:\n"
                          << nudge_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object nudge stderr:\n"
                          << nudge_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(nudge_footer_process.exit_code == 0,
                   "#1796: deleted detail-footer object nudge should exit successfully");
            expect(visual_object_deleted(footer_asset_path, "detail-footer-field-guid"),
                   "#1796: deleted detail-footer object nudge should preserve deleted state");
            expect(visual_object_property(footer_asset_path, "detail-footer-field-guid", "HPOS") == "165" &&
                       visual_object_property(footer_asset_path, "detail-footer-field-guid", "VPOS") == "340",
                   "#1796: deleted detail-footer object nudge should mutate HPOS and VPOS");
            expect_contains(nudge_footer_process.stdout_text,
                            "\"documentTitle\": \"" + title_prefix + "_deleted_footer." +
                                footer_asset_path.extension().string().substr(1) + "\"",
                            "#1796: deleted detail-footer object nudge should return refreshed layout JSON");
            if (footer_asset_path.extension() == ".lbx") {
                expect_contains(nudge_footer_process.stdout_text, "\"isLabel\": true",
                                "#1796: deleted detail-footer label object nudge should retain label identity");
            }
            expect_contains(nudge_footer_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1796: deleted detail-footer object nudge should leave live object counts unchanged");
            expect_contains(nudge_footer_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1796: deleted detail-footer object nudge should preserve deleted object counts");
            expect_contains(nudge_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1796: deleted detail-footer object nudge should preserve selected object availability");
            expect_contains(nudge_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1796: deleted detail-footer object nudge should preserve object selection kind");
            expect_contains(nudge_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1796: deleted detail-footer object nudge should preserve containing sections");
            expect_contains(nudge_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2302: deleted detail-footer object nudge should preserve live preview availability");
            expect_contains(nudge_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2302: deleted detail-footer object nudge should preserve live preview top bounds");
            expect_contains(nudge_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2302: deleted detail-footer object nudge should preserve live preview bottom bounds");
            expect_contains(nudge_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2302: deleted detail-footer object nudge should preserve live preview heights");
            expect_contains(nudge_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2302: deleted detail-footer object nudge should expose deleted preview availability");
            expect_contains(nudge_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2302: deleted detail-footer object nudge should preserve deleted preview top bounds");
            expect_contains(nudge_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 440",
                            "#2302: deleted detail-footer object nudge should refresh deleted preview bottom bounds");
            expect_contains(nudge_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 390",
                            "#2302: deleted detail-footer object nudge should refresh deleted preview heights");
            expect_contains(nudge_footer_process.stdout_text, "\"dryRun\": false",
                            "#2255: deleted detail-footer object nudge JSON should expose committed execution");
            expect_contains(nudge_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2255: deleted detail-footer object nudge JSON should expose mutation state");
            expect_contains(nudge_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2255: deleted detail-footer object nudge JSON should expose undo availability");
            expect_contains(nudge_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2255: deleted detail-footer object nudge JSON should expose nudge undo labels");
            expect_contains_in_order(
                nudge_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 40",
                    "\"sectionRelativeBottom\": 140",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"footer.total\"",
                    "\"left\": 165",
                    "\"top\": 340",
                    "\"width\": 900",
                    "\"right\": 1065",
                    "\"height\": 100",
                    "\"bottom\": 440"
                },
                "#1796: deleted detail-footer object nudge should refresh selected deleted-object metadata");
            expect_contains_in_order(
                nudge_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1796: deleted detail-footer object nudge should preserve containing-section metadata");
        };

    run_deleted_detail_header_footer_object_nudge(
        temp_root / "detail_header_footer_object_nudge_deleted_header.frx",
        temp_root / "detail_header_footer_object_nudge_deleted_footer.frx",
        "detail_header_footer_object_nudge",
        "report");
    run_deleted_detail_header_footer_object_nudge(
        temp_root / "detail_header_footer_object_nudge_deleted_header.lbx",
        temp_root / "detail_header_footer_object_nudge_deleted_footer.lbx",
        "detail_header_footer_object_nudge",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

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
