// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_deletes_and_restores_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_delete_restore_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_delete_restore =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto delete_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--delete-object",
                    "--unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (delete_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object delete stdout:\n"
                          << delete_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object delete stderr:\n"
                          << delete_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(delete_process.exit_code == 0,
                   "#1782: detail-header object delete should exit successfully");
            expect(visual_object_deleted(asset_path, "detail-header-label-guid"),
                   "#1782: detail-header object delete should mark the selected object deleted");
            expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1782: detail-header object delete should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                                "#1782: detail-header label object delete should retain label identity");
            }
            expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 1",
                            "#1782: detail-header object delete should reduce live object counts");
            expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 1",
                            "#1782: detail-header object delete should reduce placed object counts");
            expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 1",
                            "#1782: detail-header object delete should add deleted object counts");
            expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1782: detail-header object delete should preserve selected object availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1782: detail-header object delete should preserve object selection kind");
            expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1782: detail-header object delete should preserve containing-section availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1782: detail-header object delete should serialize containing-section JSON");
            expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2288: detail-header object delete should preserve live preview availability");
            expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2288: detail-header object delete should preserve live preview top bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2288: detail-header object delete should preserve live preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2288: detail-header object delete should preserve live preview heights");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2288: detail-header object delete should expose deleted preview availability");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2288: detail-header object delete should expose deleted preview top bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 170",
                            "#2288: detail-header object delete should expose deleted preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 120",
                            "#2288: detail-header object delete should expose deleted preview heights");
            expect_contains_in_order(
                delete_process.stdout_text,
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
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170"
                },
                "#1782: detail-header object delete should move the object into deleted-object JSON");
            expect_contains_in_order(
                delete_process.stdout_text,
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
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170"
                },
                "#1782: detail-header object delete should refresh selected deleted-object JSON");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1782: detail-header object delete should preserve deleted containing-section metadata");

            const auto seed_footer_delete =
                copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(seed_footer_delete.ok && visual_object_deleted(asset_path, "detail-footer-field-guid"),
                   "#1782: detail-footer object restore fixture should mark the footer object deleted");

            const auto restore_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--restore-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--json"
                },
                temp_root);

            if (restore_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object restore stdout:\n"
                          << restore_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object restore stderr:\n"
                          << restore_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(restore_process.exit_code == 0,
                   "#1782: detail-footer object restore should exit successfully");
            expect(!visual_object_deleted(asset_path, "detail-footer-field-guid"),
                   "#1782: detail-footer object restore should clear the selected object's deleted state");
            expect(visual_object_deleted(asset_path, "detail-header-label-guid"),
                   "#1782: detail-footer object restore should preserve unrelated deleted header state");
            expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1782: detail-footer object restore should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                                "#1782: detail-footer label object restore should retain label identity");
            }
            expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 1",
                            "#1782: detail-footer object restore should restore one live object while header remains deleted");
            expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 1",
                            "#1782: detail-footer object restore should restore placed object counts");
            expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                            "#1782: detail-footer object restore should leave the header object deleted");
            expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1782: detail-footer object restore should preserve selected object availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1782: detail-footer object restore should preserve object selection kind");
            expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1782: detail-footer object restore should rehydrate containing-section availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1782: detail-footer object restore should serialize containing-section JSON");
            expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2288: detail-footer object restore should preserve live preview availability");
            expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2288: detail-footer object restore should preserve live preview top bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2288: detail-footer object restore should preserve live preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2288: detail-footer object restore should preserve live preview heights");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2288: detail-footer object restore should preserve deleted preview availability");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2288: detail-footer object restore should preserve deleted preview top bounds");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 170",
                            "#2288: detail-footer object restore should preserve deleted preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 120",
                            "#2288: detail-footer object restore should preserve deleted preview heights");
            expect_contains_in_order(
                restore_process.stdout_text,
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
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 100",
                    "\"bottom\": 460"
                },
                "#1782: detail-footer object restore should refresh selected live-object JSON");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1782: detail-footer object restore should preserve containing-section object metadata");
        };

    run_detail_header_footer_object_delete_restore(temp_root / "detail_header_footer_object_delete_restore.frx",
                                                   "detail_header_footer_object_delete_restore.frx",
                                                   "report");
    run_detail_header_footer_object_delete_restore(temp_root / "detail_header_footer_object_delete_restore.lbx",
                                                   "detail_header_footer_object_delete_restore.lbx",
                                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_duplicate =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
            const std::size_t before_count = visual_object_count(asset_path);

            const auto duplicate_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--duplicate-object",
                    "--unique-id", "detail-header-label-guid",
                    "--new-unique-id", "detail-header-label-copy-guid",
                    "--json"
                },
                temp_root);

            if (duplicate_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object duplicate stdout:\n"
                          << duplicate_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object duplicate stderr:\n"
                          << duplicate_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(duplicate_header_process.exit_code == 0,
                   "#1783: detail-header object duplicate should exit successfully");
            expect(visual_object_count(asset_path) == before_count + 1U,
                   "#1783: detail-header object duplicate should append one object record");
            expect(visual_object_exists(asset_path, "detail-header-label-copy-guid"),
                   "#1783: detail-header object duplicate should persist replacement unique ids");
            expect_contains(duplicate_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1783: detail-header object duplicate should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(duplicate_header_process.stdout_text, "\"isLabel\": true",
                                "#1783: detail-header label object duplicate should retain label identity");
            }
            expect_contains(duplicate_header_process.stdout_text, "\"liveObjectCount\": 3",
                            "#1783: detail-header object duplicate should refresh live object counts");
            expect_contains(duplicate_header_process.stdout_text, "\"placedObjectCount\": 3",
                            "#1783: detail-header object duplicate should refresh placed object counts");
            expect_contains(duplicate_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1783: detail-header object duplicate should preserve selected object availability");
            expect_contains(duplicate_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1783: detail-header object duplicate should preserve object selection kind");
            expect_contains(duplicate_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1783: detail-header object duplicate should preserve containing-section availability");
            expect_contains(duplicate_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2289: detail-header object duplicate should preserve live preview availability");
            expect_contains(duplicate_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2289: detail-header object duplicate should preserve live preview top bounds");
            expect_contains(duplicate_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2289: detail-header object duplicate should preserve live preview bottom bounds");
            expect_contains(duplicate_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2289: detail-header object duplicate should preserve live preview heights");
            expect_contains(duplicate_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2289: detail-header object duplicate should not fabricate deleted preview availability");
            expect_contains_in_order(
                duplicate_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 4",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1783: detail-header object duplicate should refresh selected duplicate section metadata");
            expect_contains_in_order(
                duplicate_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 2"
                },
                "#1783: detail-header object duplicate should refresh containing-section object metadata");

            const auto duplicate_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--duplicate-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--new-unique-id", "detail-footer-field-copy-guid",
                    "--json"
                },
                temp_root);

            if (duplicate_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object duplicate stdout:\n"
                          << duplicate_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object duplicate stderr:\n"
                          << duplicate_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(duplicate_footer_process.exit_code == 0,
                   "#1783: detail-footer object duplicate should exit successfully");
            expect(visual_object_count(asset_path) == before_count + 2U,
                   "#1783: detail-footer object duplicate should append a second object record");
            expect(visual_object_exists(asset_path, "detail-footer-field-copy-guid"),
                   "#1783: detail-footer object duplicate should persist replacement unique ids");
            expect_contains(duplicate_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1783: detail-footer object duplicate should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(duplicate_footer_process.stdout_text, "\"isLabel\": true",
                                "#1783: detail-footer label object duplicate should retain label identity");
            }
            expect_contains(duplicate_footer_process.stdout_text, "\"liveObjectCount\": 4",
                            "#1783: detail-footer object duplicate should refresh live object counts");
            expect_contains(duplicate_footer_process.stdout_text, "\"placedObjectCount\": 4",
                            "#1783: detail-footer object duplicate should refresh placed object counts");
            expect_contains(duplicate_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1783: detail-footer object duplicate should preserve selected object availability");
            expect_contains(duplicate_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1783: detail-footer object duplicate should preserve object selection kind");
            expect_contains(duplicate_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1783: detail-footer object duplicate should preserve containing-section availability");
            expect_contains(duplicate_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2289: detail-footer object duplicate should preserve live preview availability");
            expect_contains(duplicate_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2289: detail-footer object duplicate should preserve live preview top bounds");
            expect_contains(duplicate_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2289: detail-footer object duplicate should preserve live preview bottom bounds");
            expect_contains(duplicate_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2289: detail-footer object duplicate should preserve live preview heights");
            expect_contains(duplicate_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2289: detail-footer object duplicate should not fabricate deleted preview availability");
            expect_contains_in_order(
                duplicate_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 5",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 100",
                    "\"bottom\": 460",
                    "\"expression\": \"footer.total\""
                },
                "#1783: detail-footer object duplicate should refresh selected duplicate section metadata");
            expect_contains_in_order(
                duplicate_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 2"
                },
                "#1783: detail-footer object duplicate should refresh containing-section object metadata");
        };

    run_detail_header_footer_object_duplicate(temp_root / "detail_header_footer_object_duplicate.frx",
                                              "detail_header_footer_object_duplicate.frx",
                                              "report");
    run_detail_header_footer_object_duplicate(temp_root / "detail_header_footer_object_duplicate.lbx",
                                              "detail_header_footer_object_duplicate.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_duplicate =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1784: deleted detail-header object duplicate fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1784: deleted detail-footer object duplicate fixture should mark the footer object deleted");
            const std::size_t before_count = visual_object_count(asset_path);

            const auto duplicate_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--duplicate-object",
                    "--unique-id", "detail-header-label-guid",
                    "--new-unique-id", "detail-header-label-copy-guid",
                    "--json"
                },
                temp_root);

            if (duplicate_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object duplicate stdout:\n"
                          << duplicate_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object duplicate stderr:\n"
                          << duplicate_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(duplicate_header_process.exit_code == 0,
                   "#1784: deleted detail-header object duplicate should exit successfully");
            expect(visual_object_count(asset_path) == before_count + 1U,
                   "#1784: deleted detail-header object duplicate should append one object record");
            expect(visual_object_exists(asset_path, "detail-header-label-copy-guid"),
                   "#1784: deleted detail-header object duplicate should persist replacement unique ids");
            expect(visual_object_deleted(asset_path, "detail-header-label-copy-guid"),
                   "#1784: deleted detail-header object duplicate should preserve deleted state");
            expect_contains(duplicate_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1784: deleted detail-header object duplicate should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(duplicate_header_process.stdout_text, "\"isLabel\": true",
                                "#1784: deleted detail-header label object duplicate should retain label identity");
            }
            expect_contains(duplicate_header_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1784: deleted detail-header object duplicate should leave live object counts unchanged");
            expect_contains(duplicate_header_process.stdout_text, "\"deletedObjectCount\": 3",
                            "#1784: deleted detail-header object duplicate should refresh deleted object counts");
            expect_contains(duplicate_header_process.stdout_text, "\"deletedPlacedObjectCount\": 3",
                            "#1784: deleted detail-header object duplicate should refresh deleted placed object counts");
            expect_contains(duplicate_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1784: deleted detail-header object duplicate should preserve selected object availability");
            expect_contains(duplicate_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1784: deleted detail-header object duplicate should preserve object selection kind");
            expect_contains(duplicate_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1784: deleted detail-header object duplicate should preserve containing sections");
            expect_contains(duplicate_header_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1784: deleted detail-header object duplicate should serialize containing-section JSON");
            expect_contains(duplicate_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2290: deleted detail-header object duplicate should preserve live preview availability");
            expect_contains(duplicate_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2290: deleted detail-header object duplicate should preserve live preview top bounds");
            expect_contains(duplicate_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2290: deleted detail-header object duplicate should preserve live preview bottom bounds");
            expect_contains(duplicate_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2290: deleted detail-header object duplicate should preserve live preview heights");
            expect_contains(duplicate_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2290: deleted detail-header object duplicate should expose deleted preview availability");
            expect_contains(duplicate_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2290: deleted detail-header object duplicate should preserve deleted preview top bounds");
            expect_contains(duplicate_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2290: deleted detail-header object duplicate should preserve deleted preview bottom bounds");
            expect_contains(duplicate_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2290: deleted detail-header object duplicate should preserve deleted preview heights");
            expect_contains_in_order(
                duplicate_header_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 4",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1784: deleted detail-header object duplicate should refresh deleted-object metadata");
            expect_contains_in_order(
                duplicate_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 4",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Header label\\\"\"",
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170"
                },
                "#1784: deleted detail-header object duplicate should refresh selected duplicate metadata");
            expect_contains_in_order(
                duplicate_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 2"
                },
                "#1784: deleted detail-header object duplicate should preserve deleted containing-section metadata");

            const auto duplicate_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--duplicate-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--new-unique-id", "detail-footer-field-copy-guid",
                    "--json"
                },
                temp_root);

            if (duplicate_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object duplicate stdout:\n"
                          << duplicate_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object duplicate stderr:\n"
                          << duplicate_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(duplicate_footer_process.exit_code == 0,
                   "#1784: deleted detail-footer object duplicate should exit successfully");
            expect(visual_object_count(asset_path) == before_count + 2U,
                   "#1784: deleted detail-footer object duplicate should append a second object record");
            expect(visual_object_exists(asset_path, "detail-footer-field-copy-guid"),
                   "#1784: deleted detail-footer object duplicate should persist replacement unique ids");
            expect(visual_object_deleted(asset_path, "detail-footer-field-copy-guid"),
                   "#1784: deleted detail-footer object duplicate should preserve deleted state");
            expect_contains(duplicate_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1784: deleted detail-footer object duplicate should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(duplicate_footer_process.stdout_text, "\"isLabel\": true",
                                "#1784: deleted detail-footer label object duplicate should retain label identity");
            }
            expect_contains(duplicate_footer_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1784: deleted detail-footer object duplicate should leave live object counts unchanged");
            expect_contains(duplicate_footer_process.stdout_text, "\"deletedObjectCount\": 4",
                            "#1784: deleted detail-footer object duplicate should refresh deleted object counts");
            expect_contains(duplicate_footer_process.stdout_text, "\"deletedPlacedObjectCount\": 4",
                            "#1784: deleted detail-footer object duplicate should refresh deleted placed object counts");
            expect_contains(duplicate_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1784: deleted detail-footer object duplicate should preserve selected object availability");
            expect_contains(duplicate_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1784: deleted detail-footer object duplicate should preserve object selection kind");
            expect_contains(duplicate_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1784: deleted detail-footer object duplicate should preserve containing sections");
            expect_contains(duplicate_footer_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1784: deleted detail-footer object duplicate should serialize containing-section JSON");
            expect_contains(duplicate_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2290: deleted detail-footer object duplicate should preserve live preview availability");
            expect_contains(duplicate_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2290: deleted detail-footer object duplicate should preserve live preview top bounds");
            expect_contains(duplicate_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2290: deleted detail-footer object duplicate should preserve live preview bottom bounds");
            expect_contains(duplicate_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2290: deleted detail-footer object duplicate should preserve live preview heights");
            expect_contains(duplicate_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2290: deleted detail-footer object duplicate should expose deleted preview availability");
            expect_contains(duplicate_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2290: deleted detail-footer object duplicate should preserve deleted preview top bounds");
            expect_contains(duplicate_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2290: deleted detail-footer object duplicate should preserve deleted preview bottom bounds");
            expect_contains(duplicate_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2290: deleted detail-footer object duplicate should preserve deleted preview heights");
            expect_contains_in_order(
                duplicate_footer_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 5",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 100",
                    "\"bottom\": 460",
                    "\"expression\": \"footer.total\""
                },
                "#1784: deleted detail-footer object duplicate should refresh deleted-object metadata");
            expect_contains_in_order(
                duplicate_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 5",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"footer.total\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 100",
                    "\"bottom\": 460"
                },
                "#1784: deleted detail-footer object duplicate should refresh selected duplicate metadata");
            expect_contains_in_order(
                duplicate_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 2"
                },
                "#1784: deleted detail-footer object duplicate should preserve deleted containing-section metadata");
        };

    run_deleted_detail_header_footer_object_duplicate(
        temp_root / "deleted_detail_header_footer_object_duplicate.frx",
        "deleted_detail_header_footer_object_duplicate.frx",
        "report");
    run_deleted_detail_header_footer_object_duplicate(
        temp_root / "deleted_detail_header_footer_object_duplicate.lbx",
        "deleted_detail_header_footer_object_duplicate.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_rename =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
            const std::size_t before_count = visual_object_count(asset_path);

            const auto rename_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--rename-object",
                    "--unique-id", "detail-header-label-guid",
                    "--new-unique-id", "detail-header-label-renamed-guid",
                    "--json"
                },
                temp_root);

            if (rename_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object rename stdout:\n"
                          << rename_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object rename stderr:\n"
                          << rename_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(rename_header_process.exit_code == 0,
                   "#1785: detail-header object rename should exit successfully");
            expect(visual_object_count(asset_path) == before_count,
                   "#1785: detail-header object rename should preserve object count");
            expect(!visual_object_exists(asset_path, "detail-header-label-guid"),
                   "#1785: detail-header object rename should remove the old unique id");
            expect(visual_object_exists(asset_path, "detail-header-label-renamed-guid"),
                   "#1785: detail-header object rename should persist replacement unique ids");
            expect_contains(rename_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1785: detail-header object rename should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(rename_header_process.stdout_text, "\"isLabel\": true",
                                "#1785: detail-header label object rename should retain label identity");
            }
            expect_contains(rename_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1785: detail-header object rename should preserve selected object availability");
            expect_contains(rename_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1785: detail-header object rename should preserve object selection kind");
            expect_contains(rename_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1785: detail-header object rename should preserve containing-section availability");
            expect_contains(rename_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2291: detail-header object rename should preserve live preview availability");
            expect_contains(rename_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2291: detail-header object rename should preserve live preview top bounds");
            expect_contains(rename_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2291: detail-header object rename should preserve live preview bottom bounds");
            expect_contains(rename_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2291: detail-header object rename should preserve live preview heights");
            expect_contains(rename_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2291: detail-header object rename should not fabricate deleted preview availability");
            expect_contains_in_order(
                rename_header_process.stdout_text,
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
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1785: detail-header object rename should preserve selected-object section metadata");
            expect_contains_in_order(
                rename_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1785: detail-header object rename should preserve containing-section object metadata");

            const auto rename_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--rename-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--new-unique-id", "detail-footer-field-renamed-guid",
                    "--json"
                },
                temp_root);

            if (rename_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object rename stdout:\n"
                          << rename_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object rename stderr:\n"
                          << rename_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(rename_footer_process.exit_code == 0,
                   "#1785: detail-footer object rename should exit successfully");
            expect(visual_object_count(asset_path) == before_count,
                   "#1785: detail-footer object rename should preserve object count");
            expect(!visual_object_exists(asset_path, "detail-footer-field-guid"),
                   "#1785: detail-footer object rename should remove the old unique id");
            expect(visual_object_exists(asset_path, "detail-footer-field-renamed-guid"),
                   "#1785: detail-footer object rename should persist replacement unique ids");
            expect_contains(rename_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1785: detail-footer object rename should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(rename_footer_process.stdout_text, "\"isLabel\": true",
                                "#1785: detail-footer label object rename should retain label identity");
            }
            expect_contains(rename_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1785: detail-footer object rename should preserve selected object availability");
            expect_contains(rename_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1785: detail-footer object rename should preserve object selection kind");
            expect_contains(rename_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1785: detail-footer object rename should preserve containing-section availability");
            expect_contains(rename_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2291: detail-footer object rename should preserve live preview availability");
            expect_contains(rename_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2291: detail-footer object rename should preserve live preview top bounds");
            expect_contains(rename_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2291: detail-footer object rename should preserve live preview bottom bounds");
            expect_contains(rename_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2291: detail-footer object rename should preserve live preview heights");
            expect_contains(rename_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2291: detail-footer object rename should not fabricate deleted preview availability");
            expect_contains_in_order(
                rename_footer_process.stdout_text,
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
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 100",
                    "\"bottom\": 460",
                    "\"expression\": \"footer.total\""
                },
                "#1785: detail-footer object rename should preserve selected-object section metadata");
            expect_contains_in_order(
                rename_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1785: detail-footer object rename should preserve containing-section object metadata");
        };

    run_detail_header_footer_object_rename(temp_root / "detail_header_footer_object_rename.frx",
                                           "detail_header_footer_object_rename.frx",
                                           "report");
    run_detail_header_footer_object_rename(temp_root / "detail_header_footer_object_rename.lbx",
                                           "detail_header_footer_object_rename.lbx",
                                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_rename =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1786: deleted detail-header object rename fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1786: deleted detail-footer object rename fixture should mark the footer object deleted");
            const std::size_t before_count = visual_object_count(asset_path);

            const auto rename_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--rename-object",
                    "--unique-id", "detail-header-label-guid",
                    "--new-unique-id", "detail-header-label-renamed-guid",
                    "--json"
                },
                temp_root);

            if (rename_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object rename stdout:\n"
                          << rename_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object rename stderr:\n"
                          << rename_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(rename_header_process.exit_code == 0,
                   "#1786: deleted detail-header object rename should exit successfully");
            expect(visual_object_count(asset_path) == before_count,
                   "#1786: deleted detail-header object rename should preserve object count");
            expect(!visual_object_exists(asset_path, "detail-header-label-guid"),
                   "#1786: deleted detail-header object rename should remove the old unique id");
            expect(visual_object_exists(asset_path, "detail-header-label-renamed-guid"),
                   "#1786: deleted detail-header object rename should persist replacement unique ids");
            expect(visual_object_deleted(asset_path, "detail-header-label-renamed-guid"),
                   "#1786: deleted detail-header object rename should preserve deleted state");
            expect_contains(rename_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1786: deleted detail-header object rename should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(rename_header_process.stdout_text, "\"isLabel\": true",
                                "#1786: deleted detail-header label object rename should retain label identity");
            }
            expect_contains(rename_header_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1786: deleted detail-header object rename should leave live object counts unchanged");
            expect_contains(rename_header_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1786: deleted detail-header object rename should preserve deleted object counts");
            expect_contains(rename_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1786: deleted detail-header object rename should preserve selected object availability");
            expect_contains(rename_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1786: deleted detail-header object rename should preserve object selection kind");
            expect_contains(rename_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1786: deleted detail-header object rename should preserve containing sections");
            expect_contains(rename_header_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1786: deleted detail-header object rename should serialize containing-section JSON");
            expect_contains(rename_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2292: deleted detail-header object rename should preserve live preview availability");
            expect_contains(rename_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2292: deleted detail-header object rename should preserve live preview top bounds");
            expect_contains(rename_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2292: deleted detail-header object rename should preserve live preview bottom bounds");
            expect_contains(rename_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2292: deleted detail-header object rename should preserve live preview heights");
            expect_contains(rename_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2292: deleted detail-header object rename should expose deleted preview availability");
            expect_contains(rename_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2292: deleted detail-header object rename should preserve deleted preview top bounds");
            expect_contains(rename_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2292: deleted detail-header object rename should preserve deleted preview bottom bounds");
            expect_contains(rename_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2292: deleted detail-header object rename should preserve deleted preview heights");
            expect_contains_in_order(
                rename_header_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1786: deleted detail-header object rename should refresh deleted-object metadata");
            expect_contains_in_order(
                rename_header_process.stdout_text,
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
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170"
                },
                "#1786: deleted detail-header object rename should refresh selected-object metadata");
            expect_contains_in_order(
                rename_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1786: deleted detail-header object rename should preserve deleted containing-section metadata");

            const auto rename_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--rename-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--new-unique-id", "detail-footer-field-renamed-guid",
                    "--json"
                },
                temp_root);

            if (rename_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object rename stdout:\n"
                          << rename_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object rename stderr:\n"
                          << rename_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(rename_footer_process.exit_code == 0,
                   "#1786: deleted detail-footer object rename should exit successfully");
            expect(visual_object_count(asset_path) == before_count,
                   "#1786: deleted detail-footer object rename should preserve object count");
            expect(!visual_object_exists(asset_path, "detail-footer-field-guid"),
                   "#1786: deleted detail-footer object rename should remove the old unique id");
            expect(visual_object_exists(asset_path, "detail-footer-field-renamed-guid"),
                   "#1786: deleted detail-footer object rename should persist replacement unique ids");
            expect(visual_object_deleted(asset_path, "detail-footer-field-renamed-guid"),
                   "#1786: deleted detail-footer object rename should preserve deleted state");
            expect_contains(rename_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1786: deleted detail-footer object rename should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(rename_footer_process.stdout_text, "\"isLabel\": true",
                                "#1786: deleted detail-footer label object rename should retain label identity");
            }
            expect_contains(rename_footer_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1786: deleted detail-footer object rename should leave live object counts unchanged");
            expect_contains(rename_footer_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1786: deleted detail-footer object rename should preserve deleted object counts");
            expect_contains(rename_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1786: deleted detail-footer object rename should preserve selected object availability");
            expect_contains(rename_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1786: deleted detail-footer object rename should preserve object selection kind");
            expect_contains(rename_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1786: deleted detail-footer object rename should preserve containing sections");
            expect_contains(rename_footer_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1786: deleted detail-footer object rename should serialize containing-section JSON");
            expect_contains(rename_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2292: deleted detail-footer object rename should preserve live preview availability");
            expect_contains(rename_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2292: deleted detail-footer object rename should preserve live preview top bounds");
            expect_contains(rename_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2292: deleted detail-footer object rename should preserve live preview bottom bounds");
            expect_contains(rename_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2292: deleted detail-footer object rename should preserve live preview heights");
            expect_contains(rename_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2292: deleted detail-footer object rename should expose deleted preview availability");
            expect_contains(rename_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2292: deleted detail-footer object rename should preserve deleted preview top bounds");
            expect_contains(rename_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2292: deleted detail-footer object rename should preserve deleted preview bottom bounds");
            expect_contains(rename_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2292: deleted detail-footer object rename should preserve deleted preview heights");
            expect_contains_in_order(
                rename_footer_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"footer.total\""
                },
                "#1786: deleted detail-footer object rename should refresh deleted-object metadata");
            expect_contains_in_order(
                rename_footer_process.stdout_text,
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
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 100",
                    "\"bottom\": 460"
                },
                "#1786: deleted detail-footer object rename should refresh selected-object metadata");
            expect_contains_in_order(
                rename_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1786: deleted detail-footer object rename should preserve deleted containing-section metadata");
        };

    run_deleted_detail_header_footer_object_rename(
        temp_root / "deleted_detail_header_footer_object_rename.frx",
        "deleted_detail_header_footer_object_rename.frx",
        "report");
    run_deleted_detail_header_footer_object_rename(
        temp_root / "deleted_detail_header_footer_object_rename.lbx",
        "deleted_detail_header_footer_object_rename.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_reorder =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto duplicate_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--duplicate-object",
                    "--unique-id", "detail-header-label-guid",
                    "--new-unique-id", "detail-header-label-copy-guid",
                    "--json"
                },
                temp_root);
            expect(duplicate_header_process.exit_code == 0 &&
                       visual_object_exists(asset_path, "detail-header-label-copy-guid"),
                   "#1787: detail-header object reorder fixture should append a header copy");

            const auto duplicate_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--duplicate-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--new-unique-id", "detail-footer-field-copy-guid",
                    "--json"
                },
                temp_root);
            expect(duplicate_footer_process.exit_code == 0 &&
                       visual_object_exists(asset_path, "detail-footer-field-copy-guid"),
                   "#1787: detail-footer object reorder fixture should append a footer copy");

            const auto reorder_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--reorder-object",
                    "--unique-id", "detail-header-label-copy-guid",
                    "--placement", "before",
                    "--target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (reorder_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object reorder stdout:\n"
                          << reorder_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object reorder stderr:\n"
                          << reorder_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(reorder_header_process.exit_code == 0,
                   "#1787: detail-header object reorder should exit successfully");
            expect_contains(reorder_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1787: detail-header object reorder should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(reorder_header_process.stdout_text, "\"isLabel\": true",
                                "#1787: detail-header label object reorder should retain label identity");
            }
            expect_contains(reorder_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1787: detail-header object reorder should preserve selected object availability");
            expect_contains(reorder_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1787: detail-header object reorder should preserve object selection kind");
            expect_contains(reorder_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1787: detail-header object reorder should preserve containing-section availability");
            expect_contains(reorder_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2293: detail-header object reorder should preserve live preview availability");
            expect_contains(reorder_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2293: detail-header object reorder should preserve live preview top bounds");
            expect_contains(reorder_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2293: detail-header object reorder should preserve live preview bottom bounds");
            expect_contains(reorder_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2293: detail-header object reorder should preserve live preview heights");
            expect_contains(reorder_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2293: detail-header object reorder should not fabricate deleted preview availability");
            expect_contains_in_order(
                reorder_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1787: detail-header object reorder should refresh selected-object section metadata");
            expect_contains_in_order(
                reorder_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 2"
                },
                "#1787: detail-header object reorder should preserve containing-section object metadata");
            expect_contains_in_order(
                reorder_header_process.stdout_text,
                {
                    "\"sectionObjectIndex\": 0",
                    "\"expression\": \"\\\"Header label\\\"\"",
                    "\"sectionObjectIndex\": 1",
                    "\"expression\": \"\\\"Header label\\\"\""
                },
                "#1787: detail-header object reorder should preserve reordered same-geometry section object order");

            const auto reorder_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--reorder-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--placement", "after",
                    "--target-unique-id", "detail-footer-field-copy-guid",
                    "--json"
                },
                temp_root);

            if (reorder_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object reorder stdout:\n"
                          << reorder_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object reorder stderr:\n"
                          << reorder_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(reorder_footer_process.exit_code == 0,
                   "#1787: detail-footer object reorder should exit successfully");
            expect_contains(reorder_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1787: detail-footer object reorder should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(reorder_footer_process.stdout_text, "\"isLabel\": true",
                                "#1787: detail-footer label object reorder should retain label identity");
            }
            expect_contains(reorder_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1787: detail-footer object reorder should preserve selected object availability");
            expect_contains(reorder_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1787: detail-footer object reorder should preserve object selection kind");
            expect_contains(reorder_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1787: detail-footer object reorder should preserve containing-section availability");
            expect_contains(reorder_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2293: detail-footer object reorder should preserve live preview availability");
            expect_contains(reorder_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2293: detail-footer object reorder should preserve live preview top bounds");
            expect_contains(reorder_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2293: detail-footer object reorder should preserve live preview bottom bounds");
            expect_contains(reorder_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2293: detail-footer object reorder should preserve live preview heights");
            expect_contains(reorder_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2293: detail-footer object reorder should not fabricate deleted preview availability");
            expect_contains_in_order(
                reorder_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 5",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 3",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 100",
                    "\"bottom\": 460",
                    "\"expression\": \"footer.total\""
                },
                "#1787: detail-footer object reorder should refresh selected-object section metadata");
            expect_contains_in_order(
                reorder_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 3",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 2"
                },
                "#1787: detail-footer object reorder should preserve containing-section object metadata");
            expect_contains_in_order(
                reorder_footer_process.stdout_text,
                {
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"sectionObjectIndex\": 0",
                    "\"expression\": \"footer.total\"",
                    "\"sectionObjectIndex\": 1",
                    "\"expression\": \"footer.total\""
                },
                "#1787: detail-footer object reorder should preserve reordered same-geometry section object order");
        };

    run_detail_header_footer_object_reorder(temp_root / "detail_header_footer_object_reorder.frx",
                                            "detail_header_footer_object_reorder.frx",
                                            "report");
    run_detail_header_footer_object_reorder(temp_root / "detail_header_footer_object_reorder.lbx",
                                            "detail_header_footer_object_reorder.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_deleted_detail_header_footer_objects_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_reorder =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto duplicate_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--duplicate-object",
                    "--unique-id", "detail-header-label-guid",
                    "--new-unique-id", "detail-header-label-copy-guid",
                    "--json"
                },
                temp_root);
            expect(duplicate_header_process.exit_code == 0 &&
                       visual_object_exists(asset_path, "detail-header-label-copy-guid"),
                   "#1788: deleted detail-header object reorder fixture should append a header copy");

            const auto duplicate_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--duplicate-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--new-unique-id", "detail-footer-field-copy-guid",
                    "--json"
                },
                temp_root);
            expect(duplicate_footer_process.exit_code == 0 &&
                       visual_object_exists(asset_path, "detail-footer-field-copy-guid"),
                   "#1788: deleted detail-footer object reorder fixture should append a footer copy");

            for (const std::size_t record_index : {1U, 3U, 4U, 5U}) {
                const auto delete_result =
                    copperfin::vfp::set_record_deleted_flag(asset_path.string(), record_index, true);
                expect(delete_result.ok && dbf_record_deleted(asset_path, record_index),
                       "#1788: deleted detail header/footer object reorder fixture should mark object records deleted");
            }

            const auto reorder_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--reorder-object",
                    "--unique-id", "detail-header-label-copy-guid",
                    "--placement", "before",
                    "--target-unique-id", "detail-header-label-guid",
                    "--json"
                },
                temp_root);

            if (reorder_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object reorder stdout:\n"
                          << reorder_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object reorder stderr:\n"
                          << reorder_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(reorder_header_process.exit_code == 0,
                   "#1788: deleted detail-header object reorder should exit successfully");
            expect(visual_object_deleted(asset_path, "detail-header-label-copy-guid"),
                   "#1788: deleted detail-header object reorder should preserve deleted state");
            expect_contains(reorder_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1788: deleted detail-header object reorder should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(reorder_header_process.stdout_text, "\"isLabel\": true",
                                "#1788: deleted detail-header label object reorder should retain label identity");
            }
            expect_contains(reorder_header_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1788: deleted detail-header object reorder should leave live object counts unchanged");
            expect_contains(reorder_header_process.stdout_text, "\"deletedObjectCount\": 4",
                            "#1788: deleted detail-header object reorder should preserve deleted object counts");
            expect_contains(reorder_header_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1788: deleted detail-header object reorder should preserve selected object availability");
            expect_contains(reorder_header_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1788: deleted detail-header object reorder should preserve object selection kind");
            expect_contains(reorder_header_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1788: deleted detail-header object reorder should preserve containing sections");
            expect_contains(reorder_header_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1788: deleted detail-header object reorder should serialize containing-section JSON");
            expect_contains(reorder_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2294: deleted detail-header object reorder should preserve live preview availability");
            expect_contains(reorder_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2294: deleted detail-header object reorder should preserve live preview top bounds");
            expect_contains(reorder_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2294: deleted detail-header object reorder should preserve live preview bottom bounds");
            expect_contains(reorder_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2294: deleted detail-header object reorder should preserve live preview heights");
            expect_contains(reorder_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2294: deleted detail-header object reorder should expose deleted preview availability");
            expect_contains(reorder_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2294: deleted detail-header object reorder should preserve deleted preview top bounds");
            expect_contains(reorder_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2294: deleted detail-header object reorder should preserve deleted preview bottom bounds");
            expect_contains(reorder_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2294: deleted detail-header object reorder should preserve deleted preview heights");
            expect_contains_in_order(
                reorder_header_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Header label\\\"\"",
                    "\"left\": 100",
                    "\"top\": 50",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 170"
                },
                "#1788: deleted detail-header object reorder should refresh selected deleted-object metadata");
            expect_contains_in_order(
                reorder_header_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 2"
                },
                "#1788: deleted detail-header object reorder should preserve deleted containing-section metadata");

            const auto reorder_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--reorder-object",
                    "--unique-id", "detail-footer-field-guid",
                    "--placement", "after",
                    "--target-unique-id", "detail-footer-field-copy-guid",
                    "--json"
                },
                temp_root);

            if (reorder_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object reorder stdout:\n"
                          << reorder_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object reorder stderr:\n"
                          << reorder_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(reorder_footer_process.exit_code == 0,
                   "#1788: deleted detail-footer object reorder should exit successfully");
            expect(visual_object_deleted(asset_path, "detail-footer-field-guid"),
                   "#1788: deleted detail-footer object reorder should preserve deleted state");
            expect_contains(reorder_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1788: deleted detail-footer object reorder should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(reorder_footer_process.stdout_text, "\"isLabel\": true",
                                "#1788: deleted detail-footer label object reorder should retain label identity");
            }
            expect_contains(reorder_footer_process.stdout_text, "\"liveObjectCount\": 0",
                            "#1788: deleted detail-footer object reorder should leave live object counts unchanged");
            expect_contains(reorder_footer_process.stdout_text, "\"deletedObjectCount\": 4",
                            "#1788: deleted detail-footer object reorder should preserve deleted object counts");
            expect_contains(reorder_footer_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1788: deleted detail-footer object reorder should preserve selected object availability");
            expect_contains(reorder_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1788: deleted detail-footer object reorder should preserve object selection kind");
            expect_contains(reorder_footer_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1788: deleted detail-footer object reorder should preserve containing sections");
            expect_contains(reorder_footer_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1788: deleted detail-footer object reorder should serialize containing-section JSON");
            expect_contains(reorder_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2294: deleted detail-footer object reorder should preserve live preview availability");
            expect_contains(reorder_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2294: deleted detail-footer object reorder should preserve live preview top bounds");
            expect_contains(reorder_footer_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2294: deleted detail-footer object reorder should preserve live preview bottom bounds");
            expect_contains(reorder_footer_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2294: deleted detail-footer object reorder should preserve live preview heights");
            expect_contains(reorder_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2294: deleted detail-footer object reorder should expose deleted preview availability");
            expect_contains(reorder_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2294: deleted detail-footer object reorder should preserve deleted preview top bounds");
            expect_contains(reorder_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2294: deleted detail-footer object reorder should preserve deleted preview bottom bounds");
            expect_contains(reorder_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2294: deleted detail-footer object reorder should preserve deleted preview heights");
            expect_contains_in_order(
                reorder_footer_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 5",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 3",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 1",
                    "\"sectionObjectCount\": 2",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"footer.total\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 100",
                    "\"bottom\": 460"
                },
                "#1788: deleted detail-footer object reorder should refresh selected deleted-object metadata");
            expect_contains_in_order(
                reorder_footer_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 3",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 2"
                },
                "#1788: deleted detail-footer object reorder should preserve deleted containing-section metadata");
        };

    run_deleted_detail_header_footer_object_reorder(
        temp_root / "deleted_detail_header_footer_object_reorder.frx",
        "deleted_detail_header_footer_object_reorder.frx",
        "report");
    run_deleted_detail_header_footer_object_reorder(
        temp_root / "deleted_detail_header_footer_object_reorder.lbx",
        "deleted_detail_header_footer_object_reorder.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
