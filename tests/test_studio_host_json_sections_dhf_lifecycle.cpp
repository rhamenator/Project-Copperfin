// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_deletes_and_restores_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_delete_restore_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_delete_restore =
        [&](const fs::path& header_delete_path,
            const fs::path& footer_delete_path,
            const fs::path& header_restore_path,
            const fs::path& footer_restore_path,
            const std::string& label) {
            const auto expect_document_identity = [&](const ProcessResult& process,
                                                      const fs::path& asset_path,
                                                      const std::string& operation_label) {
                expect_contains(process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1812: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(process.stdout_text, "\"isLabel\": true",
                                    "#1812: " + operation_label + " should retain label identity");
                }
            };

            const auto expect_selected_section_state =
                [&](const ProcessResult& process,
                    const std::string& section_title,
                    const std::string& band_kind,
                    const std::string& record_index,
                    const std::string& deleted,
                    const std::string& section_index,
                    const std::string& section_count,
                    const std::string& object_count,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#1812: " + operation_label + " should advertise selected sections");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#1812: " + operation_label + " should expose section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#1812: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1812: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2240: " + operation_label + " JSON should expose committed state");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2240: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2240: " + operation_label + " JSON should expose undo availability");
                    expect_contains(process.stdout_text, "\"undoLabel\": \"Deleted state\"",
                                    "#2240: " + operation_label + " JSON should expose the deleted-state undo label");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"title\": \"" + section_title + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": " + deleted,
                            "\"sectionIndex\": " + section_index,
                            "\"sectionCount\": " + section_count,
                            "\"objectCount\": " + object_count
                        },
                        "#1812: " + operation_label + " should refresh selected-section metadata");
                };

            const auto delete_section = [&](const fs::path& asset_path,
                                            const std::string& unique_id,
                                            std::size_t section_record_index,
                                            const std::string& section_title,
                                            const std::string& band_kind,
                                            const std::string& deleted_section_id,
                                            const std::string& live_sibling_title,
                                            const std::string& live_sibling_band_kind,
                                            const std::string& live_sibling_record_index,
                                            const std::string& orphan_object_record_index,
                                            const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

                const auto delete_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--delete-object",
                        "--unique-id", unique_id,
                        "--json"
                    },
                    temp_root);

                if (delete_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << delete_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << delete_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(delete_process.exit_code == 0,
                       "#1812: " + operation_label + " should exit successfully");
                expect(dbf_record_deleted(asset_path, section_record_index),
                       "#1812: " + operation_label + " should mark the section record deleted");
                expect_document_identity(delete_process, asset_path, operation_label);
                expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                                "#1812: " + operation_label + " should preserve the remaining live section");
                expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1812: " + operation_label + " should expose the deleted section count");
                expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 2",
                                "#1812: " + operation_label + " should keep sibling section objects placed");
                expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 0",
                                "#1812: " + operation_label + " should not orphan former section objects");
                expect_contains_in_order(
                    delete_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"" + live_sibling_title + "\"",
                        "\"bandKind\": \"" + live_sibling_band_kind + "\"",
                        "\"recordIndex\": " + live_sibling_record_index,
                        "\"deleted\": false",
                        "\"objectCount\": 1"
                    },
                    "#1812: " + operation_label + " should preserve live sibling section metadata");
                expect_contains_in_order(
                    delete_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"recordIndex\": " + std::to_string(section_record_index),
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCount\": 1"
                    },
                    "#1812: " + operation_label + " should move the section to deleted-section metadata");
                expect_contains_in_order(
                    delete_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"objects\": [",
                        "\"recordIndex\": " + orphan_object_record_index,
                        "\"deleted\": false",
                        "\"containingSectionId\": \"" + deleted_section_id + "\""
                    },
                    "#1812: " + operation_label + " should retain former section object containment inside deleted sections");
                expect_selected_section_state(delete_process,
                                              section_title,
                                              band_kind,
                                              std::to_string(section_record_index),
                                              "true",
                                              "null",
                                              "0",
                                              "1",
                                              operation_label);
            };

            const auto restore_section = [&](const fs::path& asset_path,
                                             const std::string& unique_id,
                                             std::size_t section_record_index,
                                             const std::string& section_title,
                                             const std::string& band_kind,
                                             const std::string& section_index,
                                             const std::string& containing_section_id,
                                             const std::string& placed_object_record_index,
                                             const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
                const auto delete_result =
                    copperfin::vfp::set_record_deleted_flag(asset_path.string(), section_record_index, true);
                expect(delete_result.ok && dbf_record_deleted(asset_path, section_record_index),
                       "#1812: " + operation_label + " fixture should start with a deleted section");

                const auto restore_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--restore-object",
                        "--unique-id", unique_id,
                        "--json"
                    },
                    temp_root);

                if (restore_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << restore_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << restore_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(restore_process.exit_code == 0,
                       "#1812: " + operation_label + " should exit successfully");
                expect(!dbf_record_deleted(asset_path, section_record_index),
                       "#1812: " + operation_label + " should clear the section deleted state");
                expect_document_identity(restore_process, asset_path, operation_label);
                expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                                "#1812: " + operation_label + " should restore live section counts");
                expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                                "#1812: " + operation_label + " should clear deleted section counts");
                expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                                "#1812: " + operation_label + " should restore placed object counts");
                expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 0",
                                "#1812: " + operation_label + " should clear unplaced object counts");
                expect_contains_in_order(
                    restore_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"recordIndex\": " + std::to_string(section_record_index),
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCount\": 1"
                    },
                    "#1812: " + operation_label + " should move the section back to live metadata");
                expect_contains_in_order(
                    restore_process.stdout_text,
                    {
                        "\"objects\": [",
                        "\"recordIndex\": " + placed_object_record_index,
                        "\"deleted\": false",
                        "\"containingSectionId\": \"" + containing_section_id + "\"",
                        "\"containingSectionRecordIndex\": " + std::to_string(section_record_index),
                        "\"sectionObjectIndex\": 0",
                        "\"sectionObjectCount\": 1"
                    },
                    "#1812: " + operation_label + " should restore object containing-section metadata");
                expect_selected_section_state(restore_process,
                                              section_title,
                                              band_kind,
                                              std::to_string(section_record_index),
                                              "false",
                                              section_index,
                                              "2",
                                              "1",
                                              operation_label);
            };

            delete_section(header_delete_path,
                           "detail-header-guid",
                           0U,
                           "Detail Header",
                           "detail_header",
                           "detail-header-guid",
                           "Detail Footer",
                           "detail_footer",
                           "2",
                           "1",
                           "stable detail-header section delete");
            delete_section(footer_delete_path,
                           "detail-footer-guid",
                           2U,
                           "Detail Footer",
                           "detail_footer",
                           "detail-footer-guid",
                           "Detail Header",
                           "detail_header",
                           "0",
                           "3",
                           "stable detail-footer section delete");
            restore_section(header_restore_path,
                            "detail-header-guid",
                            0U,
                            "Detail Header",
                            "detail_header",
                            "0",
                            "detail-header-guid",
                            "1",
                            "stable detail-header section restore");
            restore_section(footer_restore_path,
                            "detail-footer-guid",
                            2U,
                            "Detail Footer",
                            "detail_footer",
                            "1",
                            "detail-footer-guid",
                            "3",
                            "stable detail-footer section restore");
        };

    run_detail_header_footer_section_delete_restore(
        temp_root / "detail_header_section_delete.frx",
        temp_root / "detail_footer_section_delete.frx",
        temp_root / "detail_header_section_restore.frx",
        temp_root / "detail_footer_section_restore.frx",
        "report");
    run_detail_header_footer_section_delete_restore(
        temp_root / "detail_header_section_delete.lbx",
        temp_root / "detail_footer_section_delete.lbx",
        temp_root / "detail_header_section_restore.lbx",
        temp_root / "detail_footer_section_restore.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_section_duplicate =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto expect_document_identity = [&](const ProcessResult& process,
                                                      const fs::path& asset_path,
                                                      const std::string& operation_label) {
                expect_contains(process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1813: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(process.stdout_text, "\"isLabel\": true",
                                    "#1813: " + operation_label + " should retain label identity");
                }
            };

            const auto duplicate_section = [&](const fs::path& asset_path,
                                               const std::string& unique_id,
                                               const std::string& new_unique_id,
                                               const std::string& section_title,
                                               const std::string& band_kind,
                                               const std::string& expression,
                                               const std::string& object_code,
                                               const std::string& top,
                                               const std::string& height,
                                               const std::string& bottom,
                                               const std::string& detail_header_count,
                                               const std::string& detail_footer_count,
                                               const std::string& section_height_total,
                                               const std::string& selected_section_index,
                                               const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);

                const auto duplicate_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--duplicate-object",
                        "--unique-id", unique_id,
                        "--new-unique-id", new_unique_id,
                        "--json"
                    },
                    temp_root);

                if (duplicate_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << duplicate_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << duplicate_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(duplicate_process.exit_code == 0,
                       "#1813: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count + 1U,
                       "#1813: " + operation_label + " should append one section record");
                expect(visual_object_exists(asset_path, new_unique_id),
                       "#1813: " + operation_label + " should persist the replacement unique id");
                expect(!dbf_record_deleted(asset_path, before_count),
                       "#1813: " + operation_label + " should append a live duplicate section");
                const auto duplicated_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = before_count,
                    .object_name = {},
                    .unique_id = new_unique_id,
                    .property_name = "EXPR"
                });
                expect(duplicated_expression.ok && duplicated_expression.exists &&
                           duplicated_expression.value == expression,
                       "#1813: " + operation_label + " should preserve section expression data");

                expect_document_identity(duplicate_process, asset_path, operation_label);
                expect_contains(duplicate_process.stdout_text, "\"sectionCount\": 3",
                                "#1813: " + operation_label + " should refresh live section counts");
                expect_contains(duplicate_process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1813: " + operation_label + " should preserve deleted section counts");
                expect_contains(duplicate_process.stdout_text,
                                "\"sectionKindCounts\": [\n"
                                "        {\"kind\": \"detail_footer\", \"count\": " + detail_footer_count + "},\n"
                                "        {\"kind\": \"detail_header\", \"count\": " + detail_header_count + "}\n"
                                "      ]",
                                "#1813: " + operation_label + " should refresh detail section kind counts");
                expect_contains(duplicate_process.stdout_text, "\"sectionHeightTotal\": " + section_height_total,
                                "#1813: " + operation_label + " should refresh live section height totals");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1877: " + operation_label + " should preserve live preview availability");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1877: " + operation_label + " should preserve live preview top bounds");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsBottom\": 550",
                                "#1877: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsHeight\": 550",
                                "#1877: " + operation_label + " should preserve live preview height");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1877: " + operation_label + " should preserve deleted preview availability");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                "#1877: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                "#1877: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                "#1877: " + operation_label + " should preserve deleted preview height");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1813: " + operation_label + " should advertise selected sections");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1813: " + operation_label + " should expose section selection kind");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1813: " + operation_label + " should not select report objects");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1813: " + operation_label + " should not fabricate object sections");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1813: " + operation_label + " should not select settings");
                expect_contains(duplicate_process.stdout_text, "\"dryRun\": false",
                                "#2242: " + operation_label + " JSON should expose committed state");
                expect_contains(duplicate_process.stdout_text, "\"mutatesAsset\": true",
                                "#2242: " + operation_label + " JSON should expose mutation state");
                expect_contains(duplicate_process.stdout_text, "\"undoAvailable\": false",
                                "#2242: " + operation_label + " JSON should expose undo availability");
                expect_contains(duplicate_process.stdout_text, "\"undoLabel\": \"\"",
                                "#2242: " + operation_label + " JSON should expose empty undo labels");
                expect_contains_in_order(
                    duplicate_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + std::to_string(before_count),
                        "\"deleted\": false",
                        "\"sectionIndex\": " + selected_section_index,
                        "\"sectionCount\": 3",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1813: " + operation_label + " should select the duplicated section metadata");
                expect_contains_in_order(
                    duplicate_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"Detail Footer\"",
                        "\"bandKind\": \"detail_footer\"",
                        "\"recordIndex\": 2",
                        "\"deleted\": true"
                    },
                    "#1813: " + operation_label + " should preserve existing deleted-section metadata");
            };

            duplicate_section(header_asset_path,
                              "detail-header-guid",
                              "detail-header-copy-guid",
                              "Detail Header",
                              "detail_header",
                              "detail header expression",
                              "9",
                              "0",
                              "300",
                              "300",
                              "2",
                              "1",
                              "850",
                              "1",
                              "stable detail-header section duplicate");
            duplicate_section(footer_asset_path,
                              "detail-footer-guid",
                              "detail-footer-copy-guid",
                              "Detail Footer",
                              "detail_footer",
                              "detail footer expression",
                              "10",
                              "300",
                              "250",
                              "550",
                              "1",
                              "2",
                              "800",
                              "2",
                              "stable detail-footer section duplicate");
        };

    run_detail_header_footer_section_duplicate(
        temp_root / "detail_header_section_duplicate.frx",
        temp_root / "detail_footer_section_duplicate.frx",
        "report");
    run_detail_header_footer_section_duplicate(
        temp_root / "detail_header_section_duplicate.lbx",
        temp_root / "detail_footer_section_duplicate.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_section_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_section_duplicate =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto duplicate_deleted_section = [&](const fs::path& asset_path,
                                                       const std::string& unique_id,
                                                       const std::string& new_unique_id,
                                                       const std::string& section_title,
                                                       const std::string& band_kind,
                                                       const std::string& expression,
                                                       const std::string& object_code,
                                                       const std::string& top,
                                                       const std::string& height,
                                                       const std::string& bottom,
                                                       const std::string& deleted_height_total,
                                                       const std::string& operation_label) {
                write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);

                const auto duplicate_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--duplicate-object",
                        "--unique-id", unique_id,
                        "--new-unique-id", new_unique_id,
                        "--json"
                    },
                    temp_root);

                if (duplicate_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << duplicate_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << duplicate_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(duplicate_process.exit_code == 0,
                       "#1816: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count + 1U,
                       "#1816: " + operation_label + " should append one section record");
                expect(visual_object_exists(asset_path, new_unique_id),
                       "#1816: " + operation_label + " should persist the replacement unique id");
                expect(dbf_record_deleted(asset_path, before_count),
                       "#1816: " + operation_label + " should append a deleted section duplicate");
                const auto duplicated_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = before_count,
                    .object_name = {},
                    .unique_id = new_unique_id,
                    .property_name = "EXPR"
                });
                expect(duplicated_expression.ok && duplicated_expression.exists &&
                           duplicated_expression.value == expression,
                       "#1816: " + operation_label + " should preserve section expression data");

                expect_contains(duplicate_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1816: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(duplicate_process.stdout_text, "\"isLabel\": true",
                                    "#1816: " + operation_label + " should retain label identity");
                }
                expect_contains(duplicate_process.stdout_text, "\"sectionCount\": 1",
                                "#1816: " + operation_label + " should preserve live section counts");
                expect_contains(duplicate_process.stdout_text, "\"deletedSectionCount\": 3",
                                "#1816: " + operation_label + " should refresh deleted section counts");
                expect_contains(duplicate_process.stdout_text,
                                "\"deletedSectionHeightTotal\": " + deleted_height_total,
                                "#1816: " + operation_label + " should refresh deleted section height totals");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1874: " + operation_label + " should preserve live preview availability");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1874: " + operation_label + " should preserve live preview top bounds");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsBottom\": 500",
                                "#1874: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(duplicate_process.stdout_text, "\"previewBoundsHeight\": 500",
                                "#1874: " + operation_label + " should preserve live preview height");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1874: " + operation_label + " should preserve deleted preview availability");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                "#1874: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                "#1874: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(duplicate_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                "#1874: " + operation_label + " should preserve deleted preview height");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1816: " + operation_label + " should advertise selected sections");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1816: " + operation_label + " should expose section selection kind");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1816: " + operation_label + " should not select report objects");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1816: " + operation_label + " should not fabricate object sections");
                expect_contains(duplicate_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1816: " + operation_label + " should not select settings");
                expect_contains(duplicate_process.stdout_text, "\"dryRun\": false",
                                "#2243: " + operation_label + " JSON should expose committed state");
                expect_contains(duplicate_process.stdout_text, "\"mutatesAsset\": true",
                                "#2243: " + operation_label + " JSON should expose mutation state");
                expect_contains(duplicate_process.stdout_text, "\"undoAvailable\": false",
                                "#2243: " + operation_label + " JSON should expose undo availability");
                expect_contains(duplicate_process.stdout_text, "\"undoLabel\": \"\"",
                                "#2243: " + operation_label + " JSON should expose empty undo labels");
                expect_contains_in_order(
                    duplicate_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + std::to_string(before_count),
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1816: " + operation_label + " should select the duplicated deleted-section metadata");
                expect_contains_in_order(
                    duplicate_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"Detail\"",
                        "\"bandKind\": \"detail\"",
                        "\"recordIndex\": 0",
                        "\"deleted\": false"
                    },
                    "#1816: " + operation_label + " should preserve live Detail section metadata");
            };

            duplicate_deleted_section(header_asset_path,
                                      "deleted-detail-header-guid",
                                      "deleted-detail-header-copy-guid",
                                      "Detail Header",
                                      "detail_header",
                                      "deleted detail header expression",
                                      "9",
                                      "500",
                                      "300",
                                      "800",
                                      "850",
                                      "stable deleted detail-header section duplicate");
            duplicate_deleted_section(footer_asset_path,
                                      "deleted-detail-footer-guid",
                                      "deleted-detail-footer-copy-guid",
                                      "Detail Footer",
                                      "detail_footer",
                                      "deleted detail footer expression",
                                      "10",
                                      "800",
                                      "250",
                                      "1050",
                                      "800",
                                      "stable deleted detail-footer section duplicate");
        };

    run_deleted_detail_header_footer_section_duplicate(
        temp_root / "deleted_detail_header_section_duplicate.frx",
        temp_root / "deleted_detail_footer_section_duplicate.frx",
        "report");
    run_deleted_detail_header_footer_section_duplicate(
        temp_root / "deleted_detail_header_section_duplicate.lbx",
        temp_root / "deleted_detail_footer_section_duplicate.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_section_rename =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto rename_section = [&](const fs::path& asset_path,
                                            const std::string& unique_id,
                                            const std::string& new_unique_id,
                                            const std::string& section_title,
                                            const std::string& band_kind,
                                            const std::string& expression,
                                            const std::string& record_index,
                                            const std::string& section_index,
                                            const std::string& object_code,
                                            const std::string& top,
                                            const std::string& height,
                                            const std::string& bottom,
                                            const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);

                const auto rename_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--rename-object",
                        "--unique-id", unique_id,
                        "--new-unique-id", new_unique_id,
                        "--json"
                    },
                    temp_root);

                if (rename_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << rename_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << rename_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(rename_process.exit_code == 0,
                       "#1814: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count,
                       "#1814: " + operation_label + " should not append section records");
                expect(!visual_object_exists(asset_path, unique_id) && visual_object_exists(asset_path, new_unique_id),
                       "#1814: " + operation_label + " should replace the section unique id");
                const auto renamed_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = static_cast<std::size_t>(std::stoul(record_index)),
                    .object_name = {},
                    .unique_id = new_unique_id,
                    .property_name = "EXPR"
                });
                expect(renamed_expression.ok && renamed_expression.exists &&
                           renamed_expression.value == expression,
                       "#1814: " + operation_label + " should preserve section expression data");

                expect_contains(rename_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1814: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(rename_process.stdout_text, "\"isLabel\": true",
                                    "#1814: " + operation_label + " should retain label identity");
                }
                expect_contains(rename_process.stdout_text, "\"sectionCount\": 2",
                                "#1814: " + operation_label + " should preserve live section counts");
                expect_contains(rename_process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1814: " + operation_label + " should preserve deleted section counts");
                expect_contains(rename_process.stdout_text, "\"sectionHeightTotal\": 550",
                                "#1814: " + operation_label + " should preserve live section height totals");
                expect_contains(rename_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1878: " + operation_label + " should preserve live preview availability");
                expect_contains(rename_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1878: " + operation_label + " should preserve live preview top bounds");
                expect_contains(rename_process.stdout_text, "\"previewBoundsBottom\": 550",
                                "#1878: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(rename_process.stdout_text, "\"previewBoundsHeight\": 550",
                                "#1878: " + operation_label + " should preserve live preview height");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1878: " + operation_label + " should preserve deleted preview availability");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                "#1878: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                "#1878: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                "#1878: " + operation_label + " should preserve deleted preview height");
                expect_contains(rename_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1814: " + operation_label + " should advertise selected sections");
                expect_contains(rename_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1814: " + operation_label + " should expose section selection kind");
                expect_contains(rename_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1814: " + operation_label + " should not select report objects");
                expect_contains(rename_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1814: " + operation_label + " should not fabricate object sections");
                expect_contains(rename_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1814: " + operation_label + " should not select settings");
                expect_contains(rename_process.stdout_text, "\"dryRun\": false",
                                "#2244: " + operation_label + " JSON should expose committed state");
                expect_contains(rename_process.stdout_text, "\"mutatesAsset\": true",
                                "#2244: " + operation_label + " JSON should expose mutation state");
                expect_contains(rename_process.stdout_text, "\"undoAvailable\": true",
                                "#2244: " + operation_label + " JSON should expose undo availability");
                expect_contains(rename_process.stdout_text, "\"undoLabel\": \"Property UNIQUEID\"",
                                "#2244: " + operation_label + " JSON should expose renamed-identity undo labels");
                expect_contains_in_order(
                    rename_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1814: " + operation_label + " should select the renamed section metadata");
                expect_contains_in_order(
                    rename_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"Detail Footer\"",
                        "\"bandKind\": \"detail_footer\"",
                        "\"recordIndex\": 2",
                        "\"deleted\": true"
                    },
                    "#1814: " + operation_label + " should preserve existing deleted-section metadata");
            };

            rename_section(header_asset_path,
                           "detail-header-guid",
                           "detail-header-renamed-guid",
                           "Detail Header",
                           "detail_header",
                           "detail header expression",
                           "0",
                           "0",
                           "9",
                           "0",
                           "300",
                           "300",
                           "stable detail-header section rename");
            rename_section(footer_asset_path,
                           "detail-footer-guid",
                           "detail-footer-renamed-guid",
                           "Detail Footer",
                           "detail_footer",
                           "detail footer expression",
                           "1",
                           "1",
                           "10",
                           "300",
                           "250",
                           "550",
                           "stable detail-footer section rename");
        };

    run_detail_header_footer_section_rename(
        temp_root / "detail_header_section_rename.frx",
        temp_root / "detail_footer_section_rename.frx",
        "report");
    run_detail_header_footer_section_rename(
        temp_root / "detail_header_section_rename.lbx",
        temp_root / "detail_footer_section_rename.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_section_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_section_rename =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto rename_deleted_section = [&](const fs::path& asset_path,
                                                    const std::string& unique_id,
                                                    const std::string& new_unique_id,
                                                    const std::string& section_title,
                                                    const std::string& band_kind,
                                                    const std::string& expression,
                                                    const std::string& record_index,
                                                    const std::string& object_code,
                                                    const std::string& top,
                                                    const std::string& height,
                                                    const std::string& bottom,
                                                    const std::string& operation_label) {
                write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);
                const std::size_t section_record_index = static_cast<std::size_t>(std::stoul(record_index));

                const auto rename_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--rename-object",
                        "--unique-id", unique_id,
                        "--new-unique-id", new_unique_id,
                        "--json"
                    },
                    temp_root);

                if (rename_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << rename_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << rename_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(rename_process.exit_code == 0,
                       "#1817: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count,
                       "#1817: " + operation_label + " should not append section records");
                expect(!visual_object_exists(asset_path, unique_id) && visual_object_exists(asset_path, new_unique_id),
                       "#1817: " + operation_label + " should replace the section unique id");
                expect(dbf_record_deleted(asset_path, section_record_index),
                       "#1817: " + operation_label + " should preserve deleted state");
                const auto renamed_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = section_record_index,
                    .object_name = {},
                    .unique_id = new_unique_id,
                    .property_name = "EXPR"
                });
                expect(renamed_expression.ok && renamed_expression.exists &&
                           renamed_expression.value == expression,
                       "#1817: " + operation_label + " should preserve section expression data");

                expect_contains(rename_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1817: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(rename_process.stdout_text, "\"isLabel\": true",
                                    "#1817: " + operation_label + " should retain label identity");
                }
                expect_contains(rename_process.stdout_text, "\"sectionCount\": 1",
                                "#1817: " + operation_label + " should preserve live section counts");
                expect_contains(rename_process.stdout_text, "\"deletedSectionCount\": 2",
                                "#1817: " + operation_label + " should preserve deleted section counts");
                expect_contains(rename_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                                "#1817: " + operation_label + " should preserve deleted section height totals");
                expect_contains(rename_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1875: " + operation_label + " should preserve live preview availability");
                expect_contains(rename_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1875: " + operation_label + " should preserve live preview top bounds");
                expect_contains(rename_process.stdout_text, "\"previewBoundsBottom\": 500",
                                "#1875: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(rename_process.stdout_text, "\"previewBoundsHeight\": 500",
                                "#1875: " + operation_label + " should preserve live preview height");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1875: " + operation_label + " should preserve deleted preview availability");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                "#1875: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                "#1875: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(rename_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                "#1875: " + operation_label + " should preserve deleted preview height");
                expect_contains(rename_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1817: " + operation_label + " should advertise selected sections");
                expect_contains(rename_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1817: " + operation_label + " should expose section selection kind");
                expect_contains(rename_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1817: " + operation_label + " should not select report objects");
                expect_contains(rename_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1817: " + operation_label + " should not fabricate object sections");
                expect_contains(rename_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1817: " + operation_label + " should not select settings");
                expect_contains(rename_process.stdout_text, "\"dryRun\": false",
                                "#2245: " + operation_label + " JSON should expose committed state");
                expect_contains(rename_process.stdout_text, "\"mutatesAsset\": true",
                                "#2245: " + operation_label + " JSON should expose mutation state");
                expect_contains(rename_process.stdout_text, "\"undoAvailable\": true",
                                "#2245: " + operation_label + " JSON should expose undo availability");
                expect_contains(rename_process.stdout_text, "\"undoLabel\": \"Property UNIQUEID\"",
                                "#2245: " + operation_label + " JSON should expose renamed-identity undo labels");
                expect_contains_in_order(
                    rename_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1817: " + operation_label + " should select the renamed deleted-section metadata");
                expect_contains_in_order(
                    rename_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"Detail\"",
                        "\"bandKind\": \"detail\"",
                        "\"recordIndex\": 0",
                        "\"deleted\": false"
                    },
                    "#1817: " + operation_label + " should preserve live Detail section metadata");
            };

            rename_deleted_section(header_asset_path,
                                   "deleted-detail-header-guid",
                                   "deleted-header-renamed-guid",
                                   "Detail Header",
                                   "detail_header",
                                   "deleted detail header expression",
                                   "1",
                                   "9",
                                   "500",
                                   "300",
                                   "800",
                                   "stable deleted detail-header section rename");
            rename_deleted_section(footer_asset_path,
                                   "deleted-detail-footer-guid",
                                   "deleted-footer-renamed-guid",
                                   "Detail Footer",
                                   "detail_footer",
                                   "deleted detail footer expression",
                                   "2",
                                   "10",
                                   "800",
                                   "250",
                                   "1050",
                                   "stable deleted detail-footer section rename");
        };

    run_deleted_detail_header_footer_section_rename(
        temp_root / "deleted_detail_header_section_rename.frx",
        temp_root / "deleted_detail_footer_section_rename.frx",
        "report");
    run_deleted_detail_header_footer_section_rename(
        temp_root / "deleted_detail_header_section_rename.lbx",
        temp_root / "deleted_detail_footer_section_rename.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_section_reorder =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto reorder_section = [&](const fs::path& asset_path,
                                             const std::string& unique_id,
                                             const std::string& target_unique_id,
                                             const std::string& placement,
                                             const std::string& section_title,
                                             const std::string& band_kind,
                                             const std::string& expression,
                                             const std::string& record_index,
                                             const std::string& section_index,
                                             const std::string& object_code,
                                             const std::string& top,
                                             const std::string& height,
                                             const std::string& bottom,
                                             const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);

                const auto reorder_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--reorder-object",
                        "--unique-id", unique_id,
                        "--placement", placement,
                        "--target-unique-id", target_unique_id,
                        "--json"
                    },
                    temp_root);

                if (reorder_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << reorder_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << reorder_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(reorder_process.exit_code == 0,
                       "#1815: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count,
                       "#1815: " + operation_label + " should preserve section record count");
                expect(visual_object_exists(asset_path, unique_id),
                       "#1815: " + operation_label + " should preserve the moved section unique id");
                expect(dbf_record_deleted(asset_path, 2U),
                       "#1815: " + operation_label + " should preserve deleted-section state");
                const auto moved_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = static_cast<std::size_t>(std::stoul(record_index)),
                    .object_name = {},
                    .unique_id = unique_id,
                    .property_name = "EXPR"
                });
                expect(moved_expression.ok && moved_expression.exists &&
                           moved_expression.value == expression,
                       "#1815: " + operation_label + " should preserve section expression data");

                expect_contains(reorder_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1815: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(reorder_process.stdout_text, "\"isLabel\": true",
                                    "#1815: " + operation_label + " should retain label identity");
                }
                expect_contains(reorder_process.stdout_text, "\"sectionCount\": 2",
                                "#1815: " + operation_label + " should preserve live section counts");
                expect_contains(reorder_process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1815: " + operation_label + " should preserve deleted section counts");
                expect_contains(reorder_process.stdout_text, "\"sectionHeightTotal\": 550",
                                "#1815: " + operation_label + " should preserve live section height totals");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1879: " + operation_label + " should preserve live preview availability");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1879: " + operation_label + " should preserve live preview top bounds");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsBottom\": 550",
                                "#1879: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsHeight\": 550",
                                "#1879: " + operation_label + " should preserve live preview height");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1879: " + operation_label + " should preserve deleted preview availability");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                "#1879: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                "#1879: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                "#1879: " + operation_label + " should preserve deleted preview height");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1815: " + operation_label + " should advertise selected sections");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1815: " + operation_label + " should expose section selection kind");
                expect_contains(reorder_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1815: " + operation_label + " should not select report objects");
                expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1815: " + operation_label + " should not fabricate object sections");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1815: " + operation_label + " should not select settings");
                expect_contains(reorder_process.stdout_text, "\"dryRun\": false",
                                "#2246: " + operation_label + " JSON should expose committed state");
                expect_contains(reorder_process.stdout_text, "\"mutatesAsset\": true",
                                "#2246: " + operation_label + " JSON should expose mutation state");
                expect_contains(reorder_process.stdout_text, "\"undoAvailable\": false",
                                "#2246: " + operation_label + " JSON should expose undo availability");
                expect_contains(reorder_process.stdout_text, "\"undoLabel\": \"\"",
                                "#2246: " + operation_label + " JSON should expose empty undo labels");
                expect_contains_in_order(
                    reorder_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1815: " + operation_label + " should select the moved section metadata");
                expect_contains_in_order(
                    reorder_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"Detail Footer\"",
                        "\"bandKind\": \"detail_footer\"",
                        "\"recordIndex\": 2",
                        "\"deleted\": true"
                    },
                    "#1815: " + operation_label + " should preserve existing deleted-section metadata");
            };

            reorder_section(header_asset_path,
                            "detail-header-guid",
                            "detail-footer-guid",
                            "after",
                            "Detail Header",
                            "detail_header",
                            "detail header expression",
                            "1",
                            "0",
                            "9",
                            "0",
                            "300",
                            "300",
                            "stable detail-header section reorder");
            reorder_section(footer_asset_path,
                            "detail-footer-guid",
                            "detail-header-guid",
                            "before",
                            "Detail Footer",
                            "detail_footer",
                            "detail footer expression",
                            "0",
                            "1",
                            "10",
                            "300",
                            "250",
                            "550",
                            "stable detail-footer section reorder");
        };

    run_detail_header_footer_section_reorder(
        temp_root / "detail_header_section_reorder.frx",
        temp_root / "detail_footer_section_reorder.frx",
        "report");
    run_detail_header_footer_section_reorder(
        temp_root / "detail_header_section_reorder.lbx",
        temp_root / "detail_footer_section_reorder.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_section_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_section_reorder =
        [&](const fs::path& header_asset_path,
            const fs::path& footer_asset_path,
            const std::string& label) {
            const auto reorder_deleted_section = [&](const fs::path& asset_path,
                                                     const std::string& unique_id,
                                                     const std::string& target_unique_id,
                                                     const std::string& placement,
                                                     const std::string& section_title,
                                                     const std::string& band_kind,
                                                     const std::string& expression,
                                                     const std::string& record_index,
                                                     const std::string& object_code,
                                                     const std::string& top,
                                                     const std::string& height,
                                                     const std::string& bottom,
                                                     const std::string& operation_label) {
                write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);
                const std::size_t before_count = visual_object_count(asset_path);
                const std::size_t moved_record_index = static_cast<std::size_t>(std::stoul(record_index));

                const auto reorder_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--reorder-object",
                        "--unique-id", unique_id,
                        "--placement", placement,
                        "--target-unique-id", target_unique_id,
                        "--json"
                    },
                    temp_root);

                if (reorder_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << reorder_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << reorder_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(reorder_process.exit_code == 0,
                       "#1818: " + operation_label + " should exit successfully");
                expect(visual_object_count(asset_path) == before_count,
                       "#1818: " + operation_label + " should preserve section record count");
                expect(visual_object_exists(asset_path, unique_id),
                       "#1818: " + operation_label + " should preserve the moved section unique id");
                expect(dbf_record_deleted(asset_path, moved_record_index),
                       "#1818: " + operation_label + " should preserve deleted state");
                const auto moved_expression = copperfin::vfp::query_visual_object_property({
                    .path = asset_path.string(),
                    .record_index = moved_record_index,
                    .object_name = {},
                    .unique_id = unique_id,
                    .property_name = "EXPR"
                });
                expect(moved_expression.ok && moved_expression.exists &&
                           moved_expression.value == expression,
                       "#1818: " + operation_label + " should preserve section expression data");

                expect_contains(reorder_process.stdout_text,
                                "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1818: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(reorder_process.stdout_text, "\"isLabel\": true",
                                    "#1818: " + operation_label + " should retain label identity");
                }
                expect_contains(reorder_process.stdout_text, "\"sectionCount\": 1",
                                "#1818: " + operation_label + " should preserve live section counts");
                expect_contains(reorder_process.stdout_text, "\"deletedSectionCount\": 2",
                                "#1818: " + operation_label + " should preserve deleted section counts");
                expect_contains(reorder_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                                "#1818: " + operation_label + " should preserve deleted section height totals");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#1876: " + operation_label + " should preserve live preview availability");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsTop\": 0",
                                "#1876: " + operation_label + " should preserve live preview top bounds");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsBottom\": 500",
                                "#1876: " + operation_label + " should preserve live preview bottom bounds");
                expect_contains(reorder_process.stdout_text, "\"previewBoundsHeight\": 500",
                                "#1876: " + operation_label + " should preserve live preview height");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#1876: " + operation_label + " should preserve deleted preview availability");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                "#1876: " + operation_label + " should preserve deleted preview top bounds");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                "#1876: " + operation_label + " should preserve deleted preview bottom bounds");
                expect_contains(reorder_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                "#1876: " + operation_label + " should preserve deleted preview height");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1818: " + operation_label + " should advertise selected sections");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1818: " + operation_label + " should expose section selection kind");
                expect_contains(reorder_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1818: " + operation_label + " should not select report objects");
                expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1818: " + operation_label + " should not fabricate object sections");
                expect_contains(reorder_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1818: " + operation_label + " should not select settings");
                expect_contains(reorder_process.stdout_text, "\"dryRun\": false",
                                "#2247: " + operation_label + " JSON should expose committed state");
                expect_contains(reorder_process.stdout_text, "\"mutatesAsset\": true",
                                "#2247: " + operation_label + " JSON should expose mutation state");
                expect_contains(reorder_process.stdout_text, "\"undoAvailable\": false",
                                "#2247: " + operation_label + " JSON should expose undo availability");
                expect_contains(reorder_process.stdout_text, "\"undoLabel\": \"\"",
                                "#2247: " + operation_label + " JSON should expose empty undo labels");
                expect_contains_in_order(
                    reorder_process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1818: " + operation_label + " should select the moved deleted-section metadata");
                expect_contains_in_order(
                    reorder_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"Detail\"",
                        "\"bandKind\": \"detail\"",
                        "\"recordIndex\": 0",
                        "\"deleted\": false"
                    },
                    "#1818: " + operation_label + " should preserve live Detail section metadata");
            };

            reorder_deleted_section(header_asset_path,
                                    "deleted-detail-header-guid",
                                    "deleted-detail-footer-guid",
                                    "after",
                                    "Detail Header",
                                    "detail_header",
                                    "deleted detail header expression",
                                    "2",
                                    "9",
                                    "500",
                                    "300",
                                    "800",
                                    "stable deleted detail-header section reorder");
            reorder_deleted_section(footer_asset_path,
                                    "deleted-detail-footer-guid",
                                    "deleted-detail-header-guid",
                                    "before",
                                    "Detail Footer",
                                    "detail_footer",
                                    "deleted detail footer expression",
                                    "1",
                                    "10",
                                    "800",
                                    "250",
                                    "1050",
                                    "stable deleted detail-footer section reorder");
        };

    run_deleted_detail_header_footer_section_reorder(
        temp_root / "deleted_detail_header_section_reorder.frx",
        temp_root / "deleted_detail_footer_section_reorder.frx",
        "report");
    run_deleted_detail_header_footer_section_reorder(
        temp_root / "deleted_detail_header_section_reorder.lbx",
        temp_root / "deleted_detail_footer_section_reorder.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
