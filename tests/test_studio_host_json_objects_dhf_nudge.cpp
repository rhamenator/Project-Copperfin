// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_LAYOUT_ACTIONS_SKIP_STABLE)
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

#endif
}  // namespace cf_test_studio_host_json
