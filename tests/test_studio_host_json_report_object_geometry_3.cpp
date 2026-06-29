#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_updates_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_font_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_font_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "FONTFACE",
                    "--property-value", "Consolas",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object font update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object font update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1775: detail-header object font update should exit successfully");
            const auto updated_font = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "FONTFACE"
            });
            expect(updated_font.ok && updated_font.exists && updated_font.value == "Consolas",
                   "#1775: detail-header object font update should persist the FONTFACE memo field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1775: detail-header object font update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1775: detail-header label object font update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1775: detail-header object font update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1775: detail-header object font update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1775: detail-header object font update should preserve containing-section availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2282: detail-header object font update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2282: detail-header object font update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2282: detail-header object font update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2282: detail-header object font update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2282: detail-header object font update should not fabricate deleted preview availability");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTFACE\", \"recordIndex\": 1",
                    "\"value\": \"Consolas\""
                },
                "#1775: detail-header object font update should refresh selected-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1775: detail-header object font update should preserve containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "FONTFACE",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object font clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object font clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1775: detail-footer object font clear should exit successfully");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"FONTFACE\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 7",
                            "#1775: detail-footer object font clear should blank the FONTFACE memo field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1775: detail-footer object font clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1775: detail-footer label object font clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1775: detail-footer object font clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1775: detail-footer object font clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1775: detail-footer object font clear should preserve containing-section availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2282: detail-footer object font clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2282: detail-footer object font clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2282: detail-footer object font clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2282: detail-footer object font clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2282: detail-footer object font clear should not fabricate deleted preview availability");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1775: detail-footer object font clear should refresh selected-object highlight metadata");
            expect_not_contains(clear_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                                "#1775: detail-footer object font clear should remove stale font highlights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1775: detail-footer object font clear should preserve containing-section metadata");
        };

    run_detail_header_footer_object_font_edits(temp_root / "detail_header_footer_object_font_edits.frx",
                                               "detail_header_footer_object_font_edits.frx",
                                               "report");
    run_detail_header_footer_object_font_edits(temp_root / "detail_header_footer_object_font_edits.lbx",
                                               "detail_header_footer_object_font_edits.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_object_font_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_font_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1776: deleted detail-header object font fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1776: deleted detail-footer object font fixture should mark the footer object deleted");

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "FONTFACE",
                    "--property-value", "Consolas",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object font update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object font update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1776: deleted detail-header object font update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1776: deleted detail-header object font update should preserve deleted state");
            const auto updated_font = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "FONTFACE"
            });
            expect(updated_font.ok && updated_font.exists && updated_font.value == "Consolas",
                   "#1776: deleted detail-header object font update should persist the FONTFACE memo field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1776: deleted detail-header object font update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1776: deleted detail-header label object font update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1776: deleted detail-header object font update should preserve deleted object counts");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1776: deleted detail-header object font update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1776: deleted detail-header object font update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1776: deleted detail-header object font update should preserve containing sections");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2283: deleted detail-header object font update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2283: deleted detail-header object font update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2283: deleted detail-header object font update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2283: deleted detail-header object font update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2283: deleted detail-header object font update should expose deleted preview availability");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2283: deleted detail-header object font update should preserve deleted preview top bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2283: deleted detail-header object font update should preserve deleted preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2283: deleted detail-header object font update should preserve deleted preview heights");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTFACE\", \"recordIndex\": 1",
                    "\"value\": \"Consolas\""
                },
                "#1776: deleted detail-header object font update should refresh deleted-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
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
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTFACE\", \"recordIndex\": 1",
                    "\"value\": \"Consolas\""
                },
                "#1776: deleted detail-header object font update should refresh selected-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"deleted\": false",
                    "\"sectionIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300"
                },
                "#1776: deleted detail-header object font update should expose selected containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "FONTFACE",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object font clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object font clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1776: deleted detail-footer object font clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#1776: deleted detail-footer object font clear should preserve deleted state");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"FONTFACE\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 7",
                            "#1776: deleted detail-footer object font clear should blank the FONTFACE memo field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1776: deleted detail-footer object font clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1776: deleted detail-footer label object font clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1776: deleted detail-footer object font clear should preserve deleted object counts");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1776: deleted detail-footer object font clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1776: deleted detail-footer object font clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1776: deleted detail-footer object font clear should preserve containing sections");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2283: deleted detail-footer object font clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2283: deleted detail-footer object font clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2283: deleted detail-footer object font clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2283: deleted detail-footer object font clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2283: deleted detail-footer object font clear should expose deleted preview availability");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2283: deleted detail-footer object font clear should preserve deleted preview top bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2283: deleted detail-footer object font clear should preserve deleted preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2283: deleted detail-footer object font clear should preserve deleted preview heights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1776: deleted detail-footer object font clear should refresh deleted-object highlight metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
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
                    "\"highlightCount\": 3"
                },
                "#1776: deleted detail-footer object font clear should refresh selected-object highlight metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": false",
                    "\"sectionIndex\": 1",
                    "\"sectionCount\": 2",
                    "\"top\": 300",
                    "\"height\": 250",
                    "\"bottom\": 550"
                },
                "#1776: deleted detail-footer object font clear should expose selected containing-section metadata");
            expect_not_contains(clear_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                                "#1776: deleted detail-footer object font clear should remove stale font highlights");
        };

    run_deleted_detail_header_footer_object_font_edits(
        temp_root / "deleted_detail_header_footer_object_font_edits.frx",
        "deleted_detail_header_footer_object_font_edits.frx",
        "report");
    run_deleted_detail_header_footer_object_font_edits(
        temp_root / "deleted_detail_header_footer_object_font_edits.lbx",
        "deleted_detail_header_footer_object_font_edits.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_detail_header_footer_object_font_options_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_font_option_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_font_options =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "FONTSIZE",
                    "--property-value", "14",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object fontsize update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object fontsize update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1777: detail-header object fontsize update should exit successfully");
            const auto updated_fontsize = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "FONTSIZE"
            });
            expect(updated_fontsize.ok && updated_fontsize.exists && updated_fontsize.value == "14",
                   "#1777: detail-header object fontsize update should persist the FONTSIZE field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1777: detail-header object fontsize update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1777: detail-header label object fontsize update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1777: detail-header object fontsize update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1777: detail-header object fontsize update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1777: detail-header object fontsize update should preserve containing-section availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2284: detail-header object fontsize update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2284: detail-header object fontsize update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2284: detail-header object fontsize update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2284: detail-header object fontsize update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2284: detail-header object fontsize update should not fabricate deleted preview availability");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTSIZE\", \"recordIndex\": 1",
                    "\"value\": \"14\""
                },
                "#1777: detail-header object fontsize update should refresh selected-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1777: detail-header object fontsize update should preserve containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "MODE",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object mode clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object mode clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1777: detail-footer object mode clear should exit successfully");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                            "#1777: detail-footer object mode clear should blank the MODE field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1777: detail-footer object mode clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1777: detail-footer label object mode clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1777: detail-footer object mode clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1777: detail-footer object mode clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1777: detail-footer object mode clear should preserve containing-section availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2284: detail-footer object mode clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2284: detail-footer object mode clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2284: detail-footer object mode clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2284: detail-footer object mode clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2284: detail-footer object mode clear should not fabricate deleted preview availability");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1777: detail-footer object mode clear should refresh selected-object highlight metadata");
            expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 3",
                                "#1777: detail-footer object mode clear should remove stale mode highlights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1777: detail-footer object mode clear should preserve containing-section metadata");
        };

    run_detail_header_footer_object_font_options(temp_root / "detail_header_footer_object_font_options.frx",
                                                 "detail_header_footer_object_font_options.frx",
                                                 "report");
    run_detail_header_footer_object_font_options(temp_root / "detail_header_footer_object_font_options.lbx",
                                                 "detail_header_footer_object_font_options.lbx",
                                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_object_font_options_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_object_font_option_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_font_options =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1778: deleted detail-header object font option fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1778: deleted detail-footer object font option fixture should mark the footer object deleted");

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "FONTSIZE",
                    "--property-value", "14",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object fontsize update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object fontsize update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1778: deleted detail-header object fontsize update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1778: deleted detail-header object fontsize update should preserve deleted state");
            const auto updated_fontsize = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "FONTSIZE"
            });
            expect(updated_fontsize.ok && updated_fontsize.exists && updated_fontsize.value == "14",
                   "#1778: deleted detail-header object fontsize update should persist the FONTSIZE field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1778: deleted detail-header object fontsize update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1778: deleted detail-header label object fontsize update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1778: deleted detail-header object fontsize update should preserve deleted object counts");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1778: deleted detail-header object fontsize update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1778: deleted detail-header object fontsize update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1778: deleted detail-header object fontsize update should preserve containing sections");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1778: deleted detail-header object fontsize update should serialize containing-section JSON");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2285: deleted detail-header object fontsize update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2285: deleted detail-header object fontsize update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2285: deleted detail-header object fontsize update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2285: deleted detail-header object fontsize update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2285: deleted detail-header object fontsize update should expose deleted preview availability");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2285: deleted detail-header object fontsize update should preserve deleted preview top bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2285: deleted detail-header object fontsize update should preserve deleted preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2285: deleted detail-header object fontsize update should preserve deleted preview heights");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTSIZE\", \"recordIndex\": 1",
                    "\"value\": \"14\""
                },
                "#1778: deleted detail-header object fontsize update should refresh deleted-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
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
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTSIZE\", \"recordIndex\": 1",
                    "\"value\": \"14\""
                },
                "#1778: deleted detail-header object fontsize update should refresh selected-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1778: deleted detail-header object fontsize update should preserve deleted containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "MODE",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object mode clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object mode clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1778: deleted detail-footer object mode clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#1778: deleted detail-footer object mode clear should preserve deleted state");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                            "#1778: deleted detail-footer object mode clear should blank the MODE field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1778: deleted detail-footer object mode clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1778: deleted detail-footer label object mode clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1778: deleted detail-footer object mode clear should preserve deleted object counts");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1778: deleted detail-footer object mode clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1778: deleted detail-footer object mode clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1778: deleted detail-footer object mode clear should preserve containing sections");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1778: deleted detail-footer object mode clear should serialize containing-section JSON");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2285: deleted detail-footer object mode clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2285: deleted detail-footer object mode clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2285: deleted detail-footer object mode clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2285: deleted detail-footer object mode clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2285: deleted detail-footer object mode clear should expose deleted preview availability");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2285: deleted detail-footer object mode clear should preserve deleted preview top bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2285: deleted detail-footer object mode clear should preserve deleted preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2285: deleted detail-footer object mode clear should preserve deleted preview heights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1778: deleted detail-footer object mode clear should refresh deleted-object highlight metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
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
                    "\"highlightCount\": 3"
                },
                "#1778: deleted detail-footer object mode clear should refresh selected-object highlight metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1778: deleted detail-footer object mode clear should preserve deleted containing-section metadata");
            expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 3",
                                "#1778: deleted detail-footer object mode clear should remove stale mode highlights");
        };

    run_deleted_detail_header_footer_object_font_options(
        temp_root / "deleted_detail_header_footer_object_font_options.frx",
        "deleted_detail_header_footer_object_font_options.frx",
        "report");
    run_deleted_detail_header_footer_object_font_options(
        temp_root / "deleted_detail_header_footer_object_font_options.lbx",
        "deleted_detail_header_footer_object_font_options.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_detail_header_footer_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_geometry =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "VPOS",
                    "--property-value", "90",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object geometry update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object geometry update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1780: detail-header object geometry update should exit successfully");
            const auto updated_vpos = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "VPOS"
            });
            expect(updated_vpos.ok && updated_vpos.exists && updated_vpos.value == "90",
                   "#1780: detail-header object geometry update should persist the VPOS field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1780: detail-header object geometry update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1780: detail-header label object geometry update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1780: detail-header object geometry update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1780: detail-header object geometry update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1780: detail-header object geometry update should preserve containing-section availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2286: detail-header object geometry update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2286: detail-header object geometry update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2286: detail-header object geometry update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2286: detail-header object geometry update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2286: detail-header object geometry update should not fabricate deleted preview availability");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 90",
                    "\"sectionRelativeBottom\": 210",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 90",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 210"
                },
                "#1780: detail-header object geometry update should refresh selected-object geometry metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1780: detail-header object geometry update should preserve containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object geometry clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object geometry clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1780: detail-footer object geometry clear should exit successfully");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"HEIGHT\", \"type\": \"N\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 6",
                            "#1780: detail-footer object geometry clear should blank the HEIGHT field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1780: detail-footer object geometry clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1780: detail-footer label object geometry clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1780: detail-footer object geometry clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1780: detail-footer object geometry clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1780: detail-footer object geometry clear should preserve containing-section availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2286: detail-footer object geometry clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2286: detail-footer object geometry clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2286: detail-footer object geometry clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2286: detail-footer object geometry clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2286: detail-footer object geometry clear should not fabricate deleted preview availability");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 60",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 0",
                    "\"bottom\": 360"
                },
                "#1780: detail-footer object geometry clear should refresh selected-object geometry metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1780: detail-footer object geometry clear should preserve containing-section metadata");
        };

    run_detail_header_footer_object_geometry(temp_root / "detail_header_footer_object_geometry.frx",
                                             "detail_header_footer_object_geometry.frx",
                                             "report");
    run_detail_header_footer_object_geometry(temp_root / "detail_header_footer_object_geometry.lbx",
                                             "detail_header_footer_object_geometry.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_geometry =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1781: deleted detail-header object geometry fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1781: deleted detail-footer object geometry fixture should mark the footer object deleted");

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "VPOS",
                    "--property-value", "90",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object geometry update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object geometry update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1781: deleted detail-header object geometry update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1781: deleted detail-header object geometry update should preserve deleted state");
            const auto updated_vpos = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "VPOS"
            });
            expect(updated_vpos.ok && updated_vpos.exists && updated_vpos.value == "90",
                   "#1781: deleted detail-header object geometry update should persist the VPOS field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1781: deleted detail-header object geometry update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1781: deleted detail-header label object geometry update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1781: deleted detail-header object geometry update should preserve deleted object counts");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1781: deleted detail-header object geometry update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1781: deleted detail-header object geometry update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1781: deleted detail-header object geometry update should preserve containing sections");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1781: deleted detail-header object geometry update should serialize containing-section JSON");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2287: deleted detail-header object geometry update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2287: deleted detail-header object geometry update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2287: deleted detail-header object geometry update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2287: deleted detail-header object geometry update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2287: deleted detail-header object geometry update should expose deleted preview availability");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 90",
                            "#2287: deleted detail-header object geometry update should refresh deleted preview top bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2287: deleted detail-header object geometry update should preserve deleted preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 370",
                            "#2287: deleted detail-header object geometry update should refresh deleted preview heights");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 90",
                    "\"sectionRelativeBottom\": 210",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 90",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 210"
                },
                "#1781: deleted detail-header object geometry update should refresh deleted-object geometry metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 90",
                    "\"sectionRelativeBottom\": 210",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 90",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 210"
                },
                "#1781: deleted detail-header object geometry update should refresh selected-object geometry metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1781: deleted detail-header object geometry update should preserve deleted containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object geometry clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object geometry clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1781: deleted detail-footer object geometry clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#1781: deleted detail-footer object geometry clear should preserve deleted state");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"HEIGHT\", \"type\": \"N\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 6",
                            "#1781: deleted detail-footer object geometry clear should blank the HEIGHT field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1781: deleted detail-footer object geometry clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1781: deleted detail-footer label object geometry clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1781: deleted detail-footer object geometry clear should preserve deleted object counts");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1781: deleted detail-footer object geometry clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1781: deleted detail-footer object geometry clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1781: deleted detail-footer object geometry clear should preserve containing sections");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1781: deleted detail-footer object geometry clear should serialize containing-section JSON");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2287: deleted detail-footer object geometry clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2287: deleted detail-footer object geometry clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2287: deleted detail-footer object geometry clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2287: deleted detail-footer object geometry clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2287: deleted detail-footer object geometry clear should expose deleted preview availability");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 90",
                            "#2287: deleted detail-footer object geometry clear should preserve deleted preview top bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 360",
                            "#2287: deleted detail-footer object geometry clear should refresh deleted preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 270",
                            "#2287: deleted detail-footer object geometry clear should refresh deleted preview heights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 60",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 0",
                    "\"bottom\": 360"
                },
                "#1781: deleted detail-footer object geometry clear should refresh deleted-object geometry metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 60",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 0",
                    "\"bottom\": 360"
                },
                "#1781: deleted detail-footer object geometry clear should refresh selected-object geometry metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1781: deleted detail-footer object geometry clear should preserve deleted containing-section metadata");
        };

    run_deleted_detail_header_footer_object_geometry(temp_root / "deleted_detail_header_footer_object_geometry.frx",
                                                     "deleted_detail_header_footer_object_geometry.frx",
                                                     "report");
    run_deleted_detail_header_footer_object_geometry(temp_root / "deleted_detail_header_footer_object_geometry.lbx",
                                                     "deleted_detail_header_footer_object_geometry.lbx",
                                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clamps_negative_report_layout_dimensions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_negative_report_layout_dimension_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_negative_dimension_layout = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_negative_dimension_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " negative layout summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " negative layout summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1715: negative-dimension report/label layout JSON should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1715: negative-dimension layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1715: negative-dimension label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1715: negative-dimension live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1715: negative-dimension live layouts should keep section-origin left bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1715: negative-dimension live layouts should keep section-origin top bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 300",
                        "#1715: negative-dimension live layouts should clamp object right bounds to left plus zero width");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1715: negative-dimension live layouts should not invert bottom bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 300",
                        "#1715: negative-dimension live layouts should compute non-negative preview widths");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1715: negative-dimension live layouts should compute zero preview height");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1715: negative-dimension deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 700",
                        "#1715: negative-dimension deleted layouts should preserve deleted left bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                        "#1715: negative-dimension deleted layouts should preserve deleted top bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 700",
                        "#1715: negative-dimension deleted layouts should clamp deleted right bounds to left plus zero width");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 50",
                        "#1715: negative-dimension deleted layouts should clamp deleted bottom bounds to top plus zero height");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1715: negative-dimension deleted layouts should compute zero deleted preview width");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1715: negative-dimension deleted layouts should compute zero deleted preview height");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1715: negative-dimension layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1715: negative-dimension layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 0",
                        "#1715: negative section heights should be clamped to zero in summaries");
        expect_negative_dimension_preview_bounds(
            summary_process.stdout_text,
            "#2353: negative-dimension summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1715: negative-dimension live object selection should keep inspection non-failing");
        expect_negative_dimension_preview_bounds(
            live_process.stdout_text,
            "#2353: selected negative-dimension live object JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"left\": 300",
                "\"top\": 0",
                "\"width\": 0",
                "\"right\": 300",
                "\"height\": 0",
                "\"bottom\": 0"
            },
            "#1715: negative live object dimensions should clamp selected geometry to zero width and height");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1715: negative-dimension deleted object selection should keep inspection non-failing");
        expect_negative_dimension_preview_bounds(
            deleted_process.stdout_text,
            "#2353: selected negative-dimension deleted object JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"left\": 700",
                "\"top\": 50",
                "\"width\": 0",
                "\"right\": 700",
                "\"height\": 0",
                "\"bottom\": 50"
            },
            "#1715: negative deleted object dimensions should clamp selected geometry to zero width and height");
    };

    run_negative_dimension_layout(temp_root / "negative_dimensions.frx",
                                  "negative_dimensions.frx",
                                  "report");
    run_negative_dimension_layout(temp_root / "negative_dimensions.lbx",
                                  "negative_dimensions.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_uses_integer_portions_for_fractional_report_layout_geometry(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_fractional_report_layout_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_fractional_layout = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_fractional_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " fractional layout summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " fractional layout summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1719: fractional report/label layout numerics should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1719: fractional layout numerics should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1719: fractional numeric label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1719: fractional live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1719: fractional live layout left bounds should include the section origin");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 10",
                        "#1719: fractional section top should use the integer portion");
        expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 425",
                        "#1719: fractional live layout right bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 1010",
                        "#1719: fractional section bottom should use integer portions");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 425",
                        "#1719: fractional live layout width should use integer portions");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 1000",
                        "#1719: fractional live layout height should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1719: fractional deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 425",
                        "#1719: fractional deleted layout left bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 700",
                        "#1719: fractional deleted layout top bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 575",
                        "#1719: fractional deleted layout right bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 740",
                        "#1719: fractional deleted layout bottom bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                        "#1719: fractional deleted layout width should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 40",
                        "#1719: fractional deleted layout height should use integer portions");
        expect_fractional_geometry_preview_bounds(
            summary_process.stdout_text,
            "#2347: fractional geometry summary JSON");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1719: fractional layout numerics should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1719: fractional layout numerics should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 1000",
                        "#1719: fractional section heights should use integer portions in summaries");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1719: fractional live object selection should keep inspection non-failing");
        expect_fractional_geometry_preview_bounds(
            live_process.stdout_text,
            "#2347: selected fractional live object JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"left\": 125",
                "\"top\": 200",
                "\"width\": 300",
                "\"right\": 425",
                "\"height\": 80",
                "\"bottom\": 280"
            },
            "#1719: fractional live object geometry should use integer portions");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1719: fractional deleted object selection should keep inspection non-failing");
        expect_fractional_geometry_preview_bounds(
            deleted_process.stdout_text,
            "#2347: selected fractional deleted object JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"left\": 425",
                "\"top\": 700",
                "\"width\": 150",
                "\"right\": 575",
                "\"height\": 40",
                "\"bottom\": 740"
            },
            "#1719: fractional deleted object geometry should use integer portions");
    };

    run_fractional_layout(temp_root / "fractional_geometry.frx",
                          "fractional_geometry.frx",
                          "report");
    run_fractional_layout(temp_root / "fractional_geometry.lbx",
                          "fractional_geometry.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_defaults_missing_report_layout_geometry_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_layout_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_missing_geometry_layout = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_synthetic_report_table_for_missing_geometry_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing geometry summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing geometry summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1720: missing report/label layout geometry fields should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1720: missing geometry layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1720: missing geometry label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1720: missing geometry live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1720: missing geometry live layout left bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1720: missing geometry live layout top bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1720: missing geometry live layout right bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1720: missing geometry live layout bottom bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1720: missing geometry live layout width should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1720: missing geometry live layout height should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1720: missing geometry deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1720: missing geometry deleted layout left bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1720: missing geometry deleted layout top bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1720: missing geometry deleted layout right bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#1720: missing geometry deleted layout bottom bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1720: missing geometry deleted layout width should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1720: missing geometry deleted layout height should default to zero");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1720: missing geometry layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1720: missing geometry layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1720: missing geometry layouts should preserve section rows");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 0",
                        "#1720: missing section geometry should default to zero in summaries");
        expect_zero_available_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2360: missing-geometry summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1720: missing geometry live object selection should keep inspection non-failing");
        expect_zero_available_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2313: selected missing-geometry live object JSON");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1720: missing geometry live object should still resolve the zero-height section");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"left\": 0",
                "\"leftFieldIndex\": null",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"width\": 0",
                "\"widthFieldIndex\": null",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1720: missing live object geometry should default to zero with null field provenance");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1720: missing geometry deleted object selection should keep inspection non-failing");
        expect_zero_available_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2313: selected missing-geometry deleted object JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"left\": 0",
                "\"leftFieldIndex\": null",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"width\": 0",
                "\"widthFieldIndex\": null",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1720: missing deleted object geometry should default to zero with null field provenance");
    };

    run_missing_geometry_layout(temp_root / "missing_geometry.frx",
                                "missing_geometry.frx",
                                "report");
    run_missing_geometry_layout(temp_root / "missing_geometry.lbx",
                                "missing_geometry.lbx",
                                "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_defaults_unresolved_report_geometry_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_report_geometry_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_unresolved_geometry_memo_layout = [&](const fs::path& asset_path,
                                                         const std::string& title,
                                                         const std::string& label) {
        write_synthetic_report_table_for_unresolved_geometry_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved geometry memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved geometry memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1740: unresolved geometry memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1740: unresolved geometry memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1740: unresolved geometry memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1740: unresolved geometry memo layouts should expose defaulted preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1740: unresolved geometry memo live preview width should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1740: unresolved geometry memo live preview height should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1740: unresolved geometry memo layouts should expose deleted defaulted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1740: unresolved geometry memo deleted preview width should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1740: unresolved geometry memo deleted preview height should stay non-inverted");
        expect_zero_available_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2336: unresolved geometry memo summary JSON");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1740: unresolved geometry memo layouts should preserve section rows");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1740: unresolved geometry memo layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1740: unresolved geometry memo layouts should preserve deleted object counts");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"detail_0\"",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved section geometry memo placeholders should default to zero with field provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"objects\": [",
                "\"recordIndex\": 1",
                "\"left\": 0",
                "\"leftFieldIndex\": 3",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"width\": 0",
                "\"widthFieldIndex\": 5",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved live object geometry memo placeholders should default to zero with field provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 2",
                "\"left\": 0",
                "\"leftFieldIndex\": 3",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"width\": 0",
                "\"widthFieldIndex\": 5",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved deleted object geometry memo placeholders should default to zero with field provenance");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1740: unresolved geometry memo placeholders should not leak into summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1740: unresolved live object geometry memo selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1740: unresolved live object geometry memo selection should advertise selected objects");
        expect_zero_available_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2336: selected unresolved live geometry memo JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"left\": 0",
                "\"leftFieldIndex\": 3",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"width\": 0",
                "\"widthFieldIndex\": 5",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved live object geometry memo selection should expose zero non-inverted geometry");
        expect_not_contains(live_process.stdout_text, "<memo block",
                            "#1740: unresolved live object geometry memo placeholders should not leak into selection JSON");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1740: unresolved deleted object geometry memo selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1740: unresolved deleted object geometry memo selection should advertise selected objects");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"left\": 0",
                "\"leftFieldIndex\": 3",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"width\": 0",
                "\"widthFieldIndex\": 5",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved deleted object geometry memo selection should expose zero non-inverted geometry");
        expect_not_contains(deleted_process.stdout_text, "<memo block",
                            "#1740: unresolved deleted object geometry memo placeholders should not leak into selection JSON");
    };

    run_unresolved_geometry_memo_layout(temp_root / "unresolved_geometry_memo.frx",
                                        "unresolved_geometry_memo.frx",
                                        "report");
    run_unresolved_geometry_memo_layout(temp_root / "unresolved_geometry_memo.lbx",
                                        "unresolved_geometry_memo.lbx",
                                        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_defaults_report_sections_without_geometry_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_section_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_missing_section_geometry_layout = [&](const fs::path& asset_path,
                                                         const std::string& title,
                                                         const std::string& label) {
        write_synthetic_report_table_for_missing_section_geometry_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing section geometry summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing section geometry summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1727: missing section geometry schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1727: missing section geometry layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1727: missing section geometry label layouts should retain label identity");
        }
        expect_zero_available_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2341: missing section geometry summary JSON");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1727: missing section geometry layouts should preserve live section counts");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1727: missing section geometry layouts should preserve deleted section counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 0",
                        "#1727: missing live section geometry should default height totals to zero");
        expect_contains(summary_process.stdout_text, "\"deletedSectionHeightTotal\": 0",
                        "#1727: missing deleted section geometry should default height totals to zero");
        expect_contains(summary_process.stdout_text, "\"sectionKindCounts\": [\n        {\"kind\": \"page_header\", \"count\": 1}\n      ]",
                        "#1727: missing live section geometry should preserve band-kind counts");
        expect_contains(summary_process.stdout_text, "\"deletedSectionKindCounts\": [\n        {\"kind\": \"summary\", \"count\": 1}\n      ]",
                        "#1727: missing deleted section geometry should preserve band-kind counts");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"page_header_0\"",
                "\"title\": \"Page Header\"",
                "\"bandKind\": \"page_header\"",
                "\"expression\": \"page.header.missing.geometry\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 1",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1727: missing live section geometry should serialize zero geometry with null provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"summary_1\"",
                "\"title\": \"Summary\"",
                "\"bandKind\": \"summary\"",
                "\"expression\": \"summary.missing.geometry\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 8",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1727: missing deleted section geometry should serialize zero geometry with null provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1727: missing section geometry live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1727: missing section geometry live selection should advertise selected sections");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1727: missing section geometry live selection should expose section selection kind");
        expect_zero_available_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2341: selected missing live section geometry JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"page_header_0\"",
                "\"title\": \"Page Header\"",
                "\"bandKind\": \"page_header\"",
                "\"expression\": \"page.header.missing.geometry\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 1",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1727: missing live section geometry selection should expose zero geometry with null provenance");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1727: missing section geometry deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1727: missing section geometry deleted selection should advertise selected sections");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1727: missing section geometry deleted selection should expose section selection kind");
        expect_zero_available_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2341: selected missing deleted section geometry JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"summary_1\"",
                "\"title\": \"Summary\"",
                "\"bandKind\": \"summary\"",
                "\"expression\": \"summary.missing.geometry\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 8",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1727: missing deleted section geometry selection should expose zero geometry with null provenance");
    };

    run_missing_section_geometry_layout(temp_root / "missing_section_geometry.frx",
                                        "missing_section_geometry.frx",
                                        "report");
    run_missing_section_geometry_layout(temp_root / "missing_section_geometry.lbx",
                                        "missing_section_geometry.lbx",
                                        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
