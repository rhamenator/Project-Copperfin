// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_CLEARS_SKIP_HOST_SMOKE)
void test_studio_host_json_clears_deleted_report_visual_properties_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1871: deleted report/label clear fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_clear = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-clear",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1871: deleted report/label stable visual-property clear JSON should exit successfully");
        expect_contains(clear_process.stdout_text, "\"visualPropertyClear\": {",
                        "#1871: deleted report/label stable visual-property clear JSON should expose a clear object");
        expect_contains(clear_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#1871: deleted report/label stable visual-property clear JSON should expose affected property counts");
        expect_contains(clear_process.stdout_text, "\"dryRun\": false",
                        "#1871: deleted report/label stable visual-property clear JSON should expose committed state");
        expect_contains(clear_process.stdout_text, "\"mutatesAsset\": true",
                        "#1871: deleted report/label stable visual-property clear JSON should expose mutation state");
        expect_contains(clear_process.stdout_text, "\"undoAvailable\": true",
                        "#1871: deleted report/label stable visual-property clear JSON should expose undo availability");
        expect_contains(clear_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                        "#2157: deleted report/label stable visual-property clear JSON should expose expression undo labels");
        expect(visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1871: deleted report/label stable visual-property clear should preserve deleted state");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property clear reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property clear reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1871: deleted report/label stable visual-property clear reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1871: deleted report/label stable visual-property clear should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1871: deleted label stable visual-property clear should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2059: stable deleted report/label visual-property clear JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2059: stable deleted report/label visual-property clear JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1871: deleted report/label stable visual-property clear should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1871: deleted report/label stable visual-property clear should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1871: deleted report/label stable visual-property clear should select the cleared deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1871: deleted report/label stable visual-property clear should preserve containing-section availability");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1871: deleted report/label stable visual-property clear should serialize containing-section metadata");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1871: deleted report/label stable visual-property clear should preserve report object selection kind");
        expect_contains(reopen_process.stdout_text, "\"recordIndex\": 3",
                        "#1871: deleted report/label stable visual-property clear should preserve cleared record indexes");
        expect_contains(reopen_process.stdout_text, "\"deleted\": true",
                        "#1871: deleted report/label stable visual-property clear should preserve cleared deleted state");
        expect_contains(reopen_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1871: deleted report/label stable visual-property clear should preserve cleared object kind");
        expect_contains(reopen_process.stdout_text, "\"expression\": \"\"",
                        "#1871: deleted report/label stable visual-property clear should refresh cleared expressions");
        expect_not_contains(reopen_process.stdout_text, "\"expression\": \"middle.value\"",
                            "#1871: deleted report/label stable visual-property clear should not expose stale expressions");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"middle-field-guid\"",
                        "#1871: deleted report/label stable visual-property clear should preserve stable identities");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"expression\": \"\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1871: deleted report/label stable visual-property clear should refresh selected deleted-row section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1871: deleted report/label stable visual-property clear should expose containing detail-band metadata");

        const auto geometry_clear_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-clear",
                "--path", asset_path.string(),
                "--property-name", "HPOS",
                "--unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        if (geometry_clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry clear stdout:\n"
                      << geometry_clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry clear stderr:\n"
                      << geometry_clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_clear_process.exit_code == 0,
               "#2059: deleted report/label stable visual-property geometry clear JSON should exit successfully");
        expect_contains(geometry_clear_process.stdout_text, "\"visualPropertyClear\": {",
                        "#2059: deleted report/label stable visual-property geometry clear JSON should expose a clear object");
        expect_contains(geometry_clear_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#2059: deleted report/label stable visual-property geometry clear JSON should expose affected property counts");
        expect_contains(geometry_clear_process.stdout_text, "\"dryRun\": false",
                        "#2192: deleted report/label stable visual-property geometry clear JSON should expose committed state");
        expect_contains(geometry_clear_process.stdout_text, "\"mutatesAsset\": true",
                        "#2192: deleted report/label stable visual-property geometry clear JSON should expose mutation state");
        expect_contains(geometry_clear_process.stdout_text, "\"undoAvailable\": true",
                        "#2192: deleted report/label stable visual-property geometry clear JSON should expose undo availability");
        expect_contains(geometry_clear_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2157: deleted report/label stable visual-property geometry clear JSON should expose geometry undo labels");

        const auto geometry_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);

        if (geometry_reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry clear reopen stdout:\n"
                      << geometry_reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry clear reopen stderr:\n"
                      << geometry_reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_reopen_process.exit_code == 0,
               "#2059: deleted report/label stable visual-property geometry clear reopen should exit successfully");
        expect_contains(geometry_reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve live preview availability");
        expect_contains(geometry_reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve live preview left bounds");
        expect_contains(geometry_reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve live preview top bounds");
        expect_contains(geometry_reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve live preview right bounds");
        expect_contains(geometry_reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve live preview bottom bounds");
        expect_contains(geometry_reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve live preview width");
        expect_contains(geometry_reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve live preview height");
        expect_contains(geometry_reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should expose deleted preview availability");
        expect_contains(geometry_reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should refresh deleted preview left bounds");
        expect_contains(geometry_reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve deleted preview top bounds");
        expect_contains(geometry_reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should refresh deleted preview right bounds");
        expect_contains(geometry_reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve deleted preview bottom bounds");
        expect_contains(geometry_reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should refresh deleted preview width");
        expect_contains(geometry_reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2059: stable deleted report/label visual-property geometry clear JSON should preserve deleted preview height");
        expect_contains(geometry_reopen_process.stdout_text, "\"left\": 0",
                        "#2059: deleted report/label stable visual-property geometry clear should refresh cleared left metadata");
        expect_contains(geometry_reopen_process.stdout_text, "\"right\": 50",
                        "#2059: deleted report/label stable visual-property geometry clear should refresh cleared bounds metadata");
    };

    const auto run_deleted_report_property_clear_failure = [&](const fs::path& asset_path,
                                                               const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto missing_selector_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-clear",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(missing_selector_process.exit_code == 4,
               "#1871: deleted report/label stable visual-property clear missing selector should fail");
        expect_contains(missing_selector_process.stdout_text, "\"visualPropertyClear\": null",
                        "#1871: failed deleted report/label stable visual-property clear JSON should not expose stale clear objects");
        expect_not_contains(missing_selector_process.stdout_text, "\"dryRun\": false",
                            "#2212: failed deleted report/label stable visual-property clear JSON should not expose stale committed state");
        expect_not_contains(missing_selector_process.stdout_text, "\"mutatesAsset\": true",
                            "#2212: failed deleted report/label stable visual-property clear JSON should not expose stale mutation state");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoAvailable\": true",
                            "#2187: failed deleted report/label stable visual-property clear JSON should not advertise undo availability");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoLabel\":",
                            "#2202: failed deleted report/label stable visual-property clear JSON should not expose stale undo labels");
        expect_contains(missing_selector_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1871: failed deleted report/label stable visual-property clear JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1871: failed deleted report/label stable visual-property clear should preserve DBF state");
        (void)label;
    };

    run_deleted_report_property_clear(temp_root / "deleted_report_property_clear.frx",
                                      "deleted_report_property_clear.frx",
                                      "report");
    run_deleted_report_property_clear(temp_root / "deleted_report_property_clear.lbx",
                                      "deleted_report_property_clear.lbx",
                                      "label");
    run_deleted_report_property_clear_failure(temp_root / "deleted_report_property_clear_missing_selector.frx",
                                              "report");
    run_deleted_report_property_clear_failure(temp_root / "deleted_report_property_clear_missing_selector.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_visual_property_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_clear_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1864: deleted report/label clear-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_clear_batch = [&](const fs::path& asset_path,
                                                             const std::string& title,
                                                             const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto clear_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-clear-batch",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--unique-id", "middle-field-guid",
                "--property-name", "WIDTH",
                "--unique-id", "middle-field-guid",
                "--property-name", "EXPR",
                "--unique-id", "right-field-guid",
                "--property-name", "HPOS",
                "--unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        if (clear_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property clear-batch stdout:\n"
                      << clear_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property clear-batch stderr:\n"
                      << clear_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_batch_process.exit_code == 0,
               "#1864: deleted report/label stable visual-property clear-batch JSON should exit successfully");
        expect_contains(clear_batch_process.stdout_text, "\"visualPropertyClearBatch\": {",
                        "#1864: deleted report/label stable visual-property clear-batch JSON should expose a batch object");
        expect_contains(clear_batch_process.stdout_text, "\"affectedObjectCount\": 4",
                        "#1864: deleted report/label stable visual-property clear-batch JSON should expose affected property counts");
        expect_contains(clear_batch_process.stdout_text, "\"dryRun\": false",
                        "#1864: deleted report/label stable visual-property clear-batch JSON should expose committed state");
        expect_contains(clear_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1864: deleted report/label stable visual-property clear-batch JSON should expose mutation state");
        expect_contains(clear_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1864: deleted report/label stable visual-property clear-batch JSON should expose undo availability");
        expect_contains(clear_batch_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2158: deleted report/label stable visual-property clear-batch JSON should expose the latest undo label");
        expect(visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1864: deleted report/label stable visual-property clear-batch should preserve deleted state");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property clear-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property clear-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1864: deleted report/label stable visual-property clear-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1864: deleted report/label stable visual-property clear-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1864: deleted label stable visual-property clear-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should refresh deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 100",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should refresh deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 100",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should refresh deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2056: stable deleted report/label visual-property clear-batch JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1864: deleted report/label stable visual-property clear-batch should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1864: deleted report/label stable visual-property clear-batch should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1864: deleted report/label stable visual-property clear-batch should select the cleared deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1864: deleted report/label stable visual-property clear-batch should preserve containing-section availability");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1864: deleted report/label stable visual-property clear-batch should serialize containing-section metadata");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1864: deleted report/label stable visual-property clear-batch should preserve report object selection kind");
        expect_contains(reopen_process.stdout_text, "\"recordIndex\": 3",
                        "#1864: deleted report/label stable visual-property clear-batch should preserve cleared record indexes");
        expect_contains(reopen_process.stdout_text, "\"deleted\": true",
                        "#1864: deleted report/label stable visual-property clear-batch should preserve cleared deleted state");
        expect_contains(reopen_process.stdout_text, "\"width\": 0",
                        "#1864: deleted report/label stable visual-property clear-batch should refresh cleared width metadata");
        expect_contains(reopen_process.stdout_text, "\"right\": 100",
                        "#1864: deleted report/label stable visual-property clear-batch should refresh cleared bounds metadata");
        expect_contains(reopen_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1864: deleted report/label stable visual-property clear-batch should preserve cleared object kind");
        expect_not_contains(reopen_process.stdout_text, "\"expression\": \"middle.value\"",
                            "#1864: deleted report/label stable visual-property clear-batch should not expose stale expressions");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"middle-field-guid\"",
                        "#1864: deleted report/label stable visual-property clear-batch should preserve cleared stable identities");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"width\": 0",
                "\"right\": 100",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1864: deleted report/label stable visual-property clear-batch should refresh selected deleted-row section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1864: deleted report/label stable visual-property clear-batch should expose containing detail-band metadata");

        const auto right_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);
        expect(right_reopen_process.exit_code == 0,
               "#1864: deleted report/label stable visual-property clear-batch right-row reopen should exit successfully");
        expect_contains(right_reopen_process.stdout_text, "\"left\": 0",
                        "#1864: deleted report/label stable visual-property clear-batch should refresh cleared left metadata");
        expect_not_contains(right_reopen_process.stdout_text, "\"expression\": \"right.value\"",
                            "#1864: deleted report/label stable visual-property clear-batch should not expose stale right expressions");
        expect_contains(right_reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#1864: deleted report/label stable visual-property clear-batch should preserve right-row stable identity");
    };

    const auto run_deleted_report_property_clear_batch_rollback = [&](const fs::path& asset_path,
                                                                      const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-clear-batch",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--unique-id", "middle-field-guid",
                "--property-name", "WIDTH",
                "--unique-id", "middle-field-guid",
                "--property-name", "EXPR",
                "--unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1864: deleted report/label stable visual-property clear-batch missing selector should fail");
        expect_contains(rollback_process.stdout_text, "\"visualPropertyClearBatch\": null",
                        "#1864: failed deleted report/label stable visual-property clear-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2213: failed deleted report/label stable visual-property clear-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2213: failed deleted report/label stable visual-property clear-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2186: failed deleted report/label stable visual-property clear-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2203: failed deleted report/label stable visual-property clear-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1864: failed deleted report/label stable visual-property clear-batch JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "middle-field-guid", "WIDTH") == "50" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1864: failed deleted report/label stable visual-property clear-batch should roll back earlier property clears");
        (void)label;
    };

    run_deleted_report_property_clear_batch(temp_root / "deleted_report_property_clear_batch.frx",
                                            "deleted_report_property_clear_batch.frx",
                                            "report");
    run_deleted_report_property_clear_batch(temp_root / "deleted_report_property_clear_batch.lbx",
                                            "deleted_report_property_clear_batch.lbx",
                                            "label");
    run_deleted_report_property_clear_batch_rollback(temp_root / "deleted_report_property_clear_batch_rollback.frx",
                                                     "report");
    run_deleted_report_property_clear_batch_rollback(temp_root / "deleted_report_property_clear_batch_rollback.lbx",
                                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif

#if !defined(COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_CLEARS_ONLY)

#if !defined(COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_MOVES_SKIP_HOST_SMOKE)
void test_studio_host_json_moves_deleted_report_visual_properties_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_move_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1870: deleted report/label move fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_move = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto move_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-move",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        if (move_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property move stdout:\n"
                      << move_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property move stderr:\n"
                      << move_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(move_process.exit_code == 0,
               "#1870: deleted report/label stable visual-property move JSON should exit successfully");
        expect_contains(move_process.stdout_text, "\"visualPropertyMove\": {",
                        "#1870: deleted report/label stable visual-property move JSON should expose a move object");
        expect_contains(move_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#1870: deleted report/label stable visual-property move JSON should expose affected property counts");
        expect_contains(move_process.stdout_text, "\"dryRun\": false",
                        "#1870: deleted report/label stable visual-property move JSON should expose committed state");
        expect_contains(move_process.stdout_text, "\"mutatesAsset\": true",
                        "#1870: deleted report/label stable visual-property move JSON should expose mutation state");
        expect_contains(move_process.stdout_text, "\"undoAvailable\": true",
                        "#1870: deleted report/label stable visual-property move JSON should expose undo availability");
        expect_contains(move_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                        "#2161: deleted report/label stable visual-property move JSON should expose moved-property undo labels");
        expect(visual_object_property(asset_path, "right-field-guid", "EXPR") == "middle.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1870: deleted report/label stable visual-property move should move values without changing deleted state");

        const auto target_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (target_reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property move target reopen stdout:\n"
                      << target_reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property move target reopen stderr:\n"
                      << target_reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(target_reopen_process.exit_code == 0,
               "#1870: deleted report/label stable visual-property move target reopen should exit successfully");
        expect_contains(target_reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1870: deleted report/label stable visual-property move should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(target_reopen_process.stdout_text, "\"isLabel\": true",
                            "#1870: deleted label stable visual-property move should retain label identity");
        }
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2061: stable deleted report/label visual-property move JSON should preserve live preview availability");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2061: stable deleted report/label visual-property move JSON should preserve live preview left bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2061: stable deleted report/label visual-property move JSON should preserve live preview top bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2061: stable deleted report/label visual-property move JSON should preserve live preview right bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2061: stable deleted report/label visual-property move JSON should preserve live preview bottom bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2061: stable deleted report/label visual-property move JSON should preserve live preview width");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2061: stable deleted report/label visual-property move JSON should preserve live preview height");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2061: stable deleted report/label visual-property move JSON should preserve deleted preview availability");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2061: stable deleted report/label visual-property move JSON should preserve deleted preview left bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2061: stable deleted report/label visual-property move JSON should preserve deleted preview top bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2061: stable deleted report/label visual-property move JSON should preserve deleted preview right bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2061: stable deleted report/label visual-property move JSON should preserve deleted preview bottom bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2061: stable deleted report/label visual-property move JSON should preserve deleted preview width");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2061: stable deleted report/label visual-property move JSON should preserve deleted preview height");
        expect_contains(target_reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1870: deleted report/label stable visual-property move should preserve live sibling counts");
        expect_contains(target_reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1870: deleted report/label stable visual-property move should preserve deleted object counts");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1870: deleted report/label stable visual-property move should select the moved target row");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1870: deleted report/label stable visual-property move should preserve containing-section availability");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1870: deleted report/label stable visual-property move should serialize containing-section metadata");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1870: deleted report/label stable visual-property move should preserve report object selection kind");
        expect_contains(target_reopen_process.stdout_text, "\"recordIndex\": 4",
                        "#1870: deleted report/label stable visual-property move should preserve target record indexes");
        expect_contains(target_reopen_process.stdout_text, "\"deleted\": true",
                        "#1870: deleted report/label stable visual-property move should preserve target deleted state");
        expect_contains(target_reopen_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1870: deleted report/label stable visual-property move should preserve target object kind");
        expect_contains(target_reopen_process.stdout_text, "\"expression\": \"middle.value\"",
                        "#1870: deleted report/label stable visual-property move should refresh moved target expressions");
        expect_contains(target_reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#1870: deleted report/label stable visual-property move should preserve target stable identities");
        expect_contains_in_order(
            target_reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"right-field-guid\""
            },
            "#1870: deleted report/label stable visual-property move should refresh target deleted-row section metadata");
        expect_contains_in_order(
            target_reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1870: deleted report/label stable visual-property move should expose containing detail-band metadata");

        const auto source_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);
        expect(source_reopen_process.exit_code == 0,
               "#1870: deleted report/label stable visual-property move source reopen should exit successfully");
        expect_contains_in_order(
            source_reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"expression\": \"\"",
                "\"highlightCount\": 0"
            },
            "#1870: deleted report/label stable visual-property move should clear selected source expressions");

        write_synthetic_report_table_for_layout_distribution_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto geometry_move_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-move",
                "--path", asset_path.string(),
                "--property-name", "HPOS",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        if (geometry_move_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry move stdout:\n"
                      << geometry_move_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry move stderr:\n"
                      << geometry_move_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_move_process.exit_code == 0,
               "#2194: deleted report/label stable visual-property geometry move JSON should exit successfully");
        expect_contains(geometry_move_process.stdout_text, "\"visualPropertyMove\": {",
                        "#2194: deleted report/label stable visual-property geometry move JSON should expose a move object");
        expect_contains(geometry_move_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#2194: deleted report/label stable visual-property geometry move JSON should expose affected property counts");
        expect_contains(geometry_move_process.stdout_text, "\"dryRun\": false",
                        "#2194: deleted report/label stable visual-property geometry move JSON should expose committed state");
        expect_contains(geometry_move_process.stdout_text, "\"mutatesAsset\": true",
                        "#2194: deleted report/label stable visual-property geometry move JSON should expose mutation state");
        expect_contains(geometry_move_process.stdout_text, "\"undoAvailable\": true",
                        "#2194: deleted report/label stable visual-property geometry move JSON should expose undo availability");
        expect_contains(geometry_move_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2194: deleted report/label stable visual-property geometry move JSON should expose geometry undo labels");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS").empty() &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "175" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#2194: deleted report/label stable visual-property geometry move should move geometry without changing deleted state");

        const auto geometry_target_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (geometry_target_reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry move target reopen stdout:\n"
                      << geometry_target_reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry move target reopen stderr:\n"
                      << geometry_target_reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_target_reopen_process.exit_code == 0,
               "#2194: deleted report/label stable visual-property geometry move target reopen should exit successfully");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2194: deleted report/label stable visual-property geometry move should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(geometry_target_reopen_process.stdout_text, "\"isLabel\": true",
                            "#2194: deleted label stable visual-property geometry move should retain label identity");
        }
        expect_contains(geometry_target_reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2194: deleted report/label stable visual-property geometry move should select the moved target row");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2194: deleted report/label stable visual-property geometry move should preserve report object selection kind");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"recordIndex\": 4",
                        "#2194: deleted report/label stable visual-property geometry move should preserve target record indexes");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"deleted\": true",
                        "#2194: deleted report/label stable visual-property geometry move should preserve target deleted state");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"left\": 175",
                        "#2194: deleted report/label stable visual-property geometry move should refresh moved target left metadata");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"right\": 225",
                        "#2194: deleted report/label stable visual-property geometry move should refresh moved target bounds metadata");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#2194: deleted report/label stable visual-property geometry move should preserve target stable identities");
    };

    const auto run_deleted_report_property_move_failure = [&](const fs::path& asset_path,
                                                              const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto missing_selector_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-move",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--source-unique-id", "missing-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        expect(missing_selector_process.exit_code == 4,
               "#1870: deleted report/label stable visual-property move missing selector should fail");
        expect_contains(missing_selector_process.stdout_text, "\"visualPropertyMove\": null",
                        "#1870: failed deleted report/label stable visual-property move JSON should not expose stale move objects");
        expect_not_contains(missing_selector_process.stdout_text, "\"dryRun\": false",
                            "#2216: failed deleted report/label stable visual-property move JSON should not expose stale committed state");
        expect_not_contains(missing_selector_process.stdout_text, "\"mutatesAsset\": true",
                            "#2216: failed deleted report/label stable visual-property move JSON should not expose stale mutation state");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoAvailable\": true",
                            "#2190: failed deleted report/label stable visual-property move JSON should not advertise undo availability");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoLabel\":",
                            "#2206: failed deleted report/label stable visual-property move JSON should not expose stale undo labels");
        expect_contains(missing_selector_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1870: failed deleted report/label stable visual-property move JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1870: failed deleted report/label stable visual-property move should preserve DBF state");
        (void)label;
    };

    run_deleted_report_property_move(temp_root / "deleted_report_property_move.frx",
                                     "deleted_report_property_move.frx",
                                     "report");
    run_deleted_report_property_move(temp_root / "deleted_report_property_move.lbx",
                                     "deleted_report_property_move.lbx",
                                     "label");
    run_deleted_report_property_move_failure(temp_root / "deleted_report_property_move_missing_selector.frx",
                                             "report");
    run_deleted_report_property_move_failure(temp_root / "deleted_report_property_move_missing_selector.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_deleted_report_visual_property_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_move_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1867: deleted report/label move-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_move_batch = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto move_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-move-batch",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        if (move_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property move-batch stdout:\n"
                      << move_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property move-batch stderr:\n"
                      << move_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(move_batch_process.exit_code == 0,
               "#1867: deleted report/label stable visual-property move-batch JSON should exit successfully");
        expect_contains(move_batch_process.stdout_text, "\"visualPropertyMoveBatch\": {",
                        "#1867: deleted report/label stable visual-property move-batch JSON should expose a batch object");
        expect_contains(move_batch_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#1867: deleted report/label stable visual-property move-batch JSON should expose affected property counts");
        expect_contains(move_batch_process.stdout_text, "\"dryRun\": false",
                        "#1867: deleted report/label stable visual-property move-batch JSON should expose committed state");
        expect_contains(move_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1867: deleted report/label stable visual-property move-batch JSON should expose mutation state");
        expect_contains(move_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1867: deleted report/label stable visual-property move-batch JSON should expose undo availability");
        expect_contains(move_batch_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                        "#2162: deleted report/label stable visual-property move-batch JSON should expose moved-property undo labels");
        expect(visual_object_property(asset_path, "right-field-guid", "EXPR") == "middle.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1867: deleted report/label stable visual-property move-batch should move values without changing deleted state");

        const auto target_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (target_reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property move-batch target reopen stdout:\n"
                      << target_reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property move-batch target reopen stderr:\n"
                      << target_reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(target_reopen_process.exit_code == 0,
               "#1867: deleted report/label stable visual-property move-batch target reopen should exit successfully");
        expect_contains(target_reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1867: deleted report/label stable visual-property move-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(target_reopen_process.stdout_text, "\"isLabel\": true",
                            "#1867: deleted label stable visual-property move-batch should retain label identity");
        }
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve live preview availability");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve live preview left bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve live preview top bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve live preview right bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve live preview bottom bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve live preview width");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve live preview height");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve deleted preview availability");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve deleted preview left bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve deleted preview top bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve deleted preview right bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve deleted preview width");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2058: stable deleted report/label visual-property move-batch JSON should preserve deleted preview height");
        expect_contains(target_reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1867: deleted report/label stable visual-property move-batch should preserve live sibling counts");
        expect_contains(target_reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1867: deleted report/label stable visual-property move-batch should preserve deleted object counts");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1867: deleted report/label stable visual-property move-batch should select the moved target row");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1867: deleted report/label stable visual-property move-batch should preserve containing-section availability");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1867: deleted report/label stable visual-property move-batch should serialize containing-section metadata");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1867: deleted report/label stable visual-property move-batch should preserve report object selection kind");
        expect_contains(target_reopen_process.stdout_text, "\"recordIndex\": 4",
                        "#1867: deleted report/label stable visual-property move-batch should preserve target record indexes");
        expect_contains(target_reopen_process.stdout_text, "\"deleted\": true",
                        "#1867: deleted report/label stable visual-property move-batch should preserve target deleted state");
        expect_contains(target_reopen_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1867: deleted report/label stable visual-property move-batch should preserve target object kind");
        expect_contains(target_reopen_process.stdout_text, "\"expression\": \"middle.value\"",
                        "#1867: deleted report/label stable visual-property move-batch should refresh moved target expressions");
        expect_contains(target_reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#1867: deleted report/label stable visual-property move-batch should preserve target stable identities");
        expect_contains_in_order(
            target_reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"right-field-guid\""
            },
            "#1867: deleted report/label stable visual-property move-batch should refresh target deleted-row section metadata");
        expect_contains_in_order(
            target_reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1867: deleted report/label stable visual-property move-batch should expose containing detail-band metadata");

        const auto source_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);
        expect(source_reopen_process.exit_code == 0,
               "#1867: deleted report/label stable visual-property move-batch source reopen should exit successfully");
        expect_contains_in_order(
            source_reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"expression\": \"\"",
                "\"highlightCount\": 0"
            },
            "#1867: deleted report/label stable visual-property move-batch should clear selected source expressions");

        write_synthetic_report_table_for_layout_distribution_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto geometry_move_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-move-batch",
                "--path", asset_path.string(),
                "--property-name", "HPOS",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        if (geometry_move_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry move-batch stdout:\n"
                      << geometry_move_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry move-batch stderr:\n"
                      << geometry_move_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_move_batch_process.exit_code == 0,
               "#2196: deleted report/label stable visual-property geometry move-batch JSON should exit successfully");
        expect_contains(geometry_move_batch_process.stdout_text, "\"visualPropertyMoveBatch\": {",
                        "#2196: deleted report/label stable visual-property geometry move-batch JSON should expose a batch object");
        expect_contains(geometry_move_batch_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#2196: deleted report/label stable visual-property geometry move-batch JSON should expose affected property counts");
        expect_contains(geometry_move_batch_process.stdout_text, "\"dryRun\": false",
                        "#2196: deleted report/label stable visual-property geometry move-batch JSON should expose committed state");
        expect_contains(geometry_move_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#2196: deleted report/label stable visual-property geometry move-batch JSON should expose mutation state");
        expect_contains(geometry_move_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#2196: deleted report/label stable visual-property geometry move-batch JSON should expose undo availability");
        expect_contains(geometry_move_batch_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2196: deleted report/label stable visual-property geometry move-batch JSON should expose geometry undo labels");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS").empty() &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "175" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#2196: deleted report/label stable visual-property geometry move-batch should move geometry without changing deleted state");

        const auto geometry_target_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (geometry_target_reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry move-batch target reopen stdout:\n"
                      << geometry_target_reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry move-batch target reopen stderr:\n"
                      << geometry_target_reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_target_reopen_process.exit_code == 0,
               "#2196: deleted report/label stable visual-property geometry move-batch target reopen should exit successfully");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2196: deleted report/label stable visual-property geometry move-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(geometry_target_reopen_process.stdout_text, "\"isLabel\": true",
                            "#2196: deleted label stable visual-property geometry move-batch should retain label identity");
        }
        expect_contains(geometry_target_reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2196: deleted report/label stable visual-property geometry move-batch should select the moved target row");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2196: deleted report/label stable visual-property geometry move-batch should preserve report object selection kind");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"recordIndex\": 4",
                        "#2196: deleted report/label stable visual-property geometry move-batch should preserve target record indexes");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"deleted\": true",
                        "#2196: deleted report/label stable visual-property geometry move-batch should preserve target deleted state");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"left\": 175",
                        "#2196: deleted report/label stable visual-property geometry move-batch should refresh moved target left metadata");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"right\": 225",
                        "#2196: deleted report/label stable visual-property geometry move-batch should refresh moved target bounds metadata");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#2196: deleted report/label stable visual-property geometry move-batch should preserve target stable identities");
    };

    const auto run_deleted_report_property_move_batch_rollback = [&](const fs::path& asset_path,
                                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-move-batch",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--property-name", "EXPR",
                "--source-unique-id", "missing-guid",
                "--target-unique-id", "left-field-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1867: deleted report/label stable visual-property move-batch missing selector should fail");
        expect_contains(rollback_process.stdout_text, "\"visualPropertyMoveBatch\": null",
                        "#1867: failed deleted report/label stable visual-property move-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2217: failed deleted report/label stable visual-property move-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2217: failed deleted report/label stable visual-property move-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2191: failed deleted report/label stable visual-property move-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2207: failed deleted report/label stable visual-property move-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1867: failed deleted report/label stable visual-property move-batch JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.value" &&
                   visual_object_property(asset_path, "left-field-guid", "EXPR") == "left.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1867: failed deleted report/label stable visual-property move-batch should roll back earlier moves");

        write_synthetic_report_table_for_layout_distribution_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto geometry_rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-move-batch",
                "--path", asset_path.string(),
                "--property-name", "HPOS",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--property-name", "HPOS",
                "--source-unique-id", "missing-guid",
                "--target-unique-id", "left-field-guid",
                "--json"
            },
            temp_root);

        expect(geometry_rollback_process.exit_code == 4,
               "#2198: deleted report/label stable visual-property geometry move-batch missing selector should fail");
        expect_contains(geometry_rollback_process.stdout_text, "\"visualPropertyMoveBatch\": null",
                        "#2198: failed deleted report/label stable visual-property geometry move-batch JSON should not expose stale batch objects");
        expect_not_contains(geometry_rollback_process.stdout_text, "\"dryRun\": false",
                            "#2217: failed deleted report/label stable visual-property geometry move-batch JSON should not expose stale committed state");
        expect_not_contains(geometry_rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2217: failed deleted report/label stable visual-property geometry move-batch JSON should not expose stale mutation state");
        expect_not_contains(geometry_rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2198: failed deleted report/label stable visual-property geometry move-batch JSON should not advertise undo availability");
        expect_not_contains(geometry_rollback_process.stdout_text, "\"undoLabel\":",
                            "#2207: failed deleted report/label stable visual-property geometry move-batch JSON should not expose stale undo labels");
        expect_contains(geometry_rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#2198: failed deleted report/label stable visual-property geometry move-batch JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS") == "175" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "700" &&
                   visual_object_property(asset_path, "left-field-guid", "HPOS") == "100" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#2198: failed deleted report/label stable visual-property geometry move-batch should roll back earlier geometry moves");
        (void)label;
    };

    run_deleted_report_property_move_batch(temp_root / "deleted_report_property_move_batch.frx",
                                           "deleted_report_property_move_batch.frx",
                                           "report");
    run_deleted_report_property_move_batch(temp_root / "deleted_report_property_move_batch.lbx",
                                           "deleted_report_property_move_batch.lbx",
                                           "label");
    run_deleted_report_property_move_batch_rollback(temp_root / "deleted_report_property_move_batch_rollback.frx",
                                                    "report");
    run_deleted_report_property_move_batch_rollback(temp_root / "deleted_report_property_move_batch_rollback.lbx",
                                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif

#if !defined(COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_MOVES_ONLY)

#if !defined(COPPERFIN_DELETED_REPORT_VISUAL_PROPERTY_REJECTIONS_SKIP_HOST_SMOKE)
void test_studio_host_json_rejects_deleted_report_visual_property_rename_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_rename_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1872: deleted report/label rename fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_rename_rejection = [&](const fs::path& asset_path,
                                                                  const std::string& title,
                                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rename_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-rename",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--new-property-name", "DisplayExpr",
                "--unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        if (rename_process.exit_code != 4) {
            std::cerr << "studio host " << label << " stable deleted report property rename stdout:\n"
                      << rename_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property rename stderr:\n"
                      << rename_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_process.exit_code == 4,
               "#1872: deleted report/label stable visual-property rename should reject direct FRX/LBX fields");
        expect_contains(rename_process.stdout_text, "\"visualPropertyRename\": null",
                        "#1872: failed deleted report/label stable visual-property rename JSON should not expose stale rename objects");
        expect_not_contains(rename_process.stdout_text, "\"dryRun\": false",
                            "#2218: failed deleted report/label stable visual-property rename JSON should not expose stale committed state");
        expect_not_contains(rename_process.stdout_text, "\"mutatesAsset\": true",
                            "#2218: failed deleted report/label stable visual-property rename JSON should not expose stale mutation state");
        expect_not_contains(rename_process.stdout_text, "\"undoAvailable\": true",
                            "#2163: failed deleted report/label stable visual-property rename JSON should not advertise undo availability");
        expect_not_contains(rename_process.stdout_text, "\"undoLabel\":",
                            "#2208: failed deleted report/label stable visual-property rename JSON should not expose stale undo labels");
        expect_contains(rename_process.stdout_text, "Direct DBF-backed fields cannot be renamed per object.",
                        "#1872: failed deleted report/label stable visual-property rename JSON should report direct-field errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "middle-field-guid", "DisplayExpr").empty() &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1872: failed deleted report/label stable visual-property rename should preserve DBF-backed fields");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property rename reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property rename reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1872: deleted report/label stable visual-property rename rejection reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1872: deleted report/label stable visual-property rename rejection should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1872: deleted label stable visual-property rename rejection should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2062: stable deleted report/label visual-property rename rejection JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1872: deleted report/label stable visual-property rename rejection should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1872: deleted report/label stable visual-property rename rejection should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1872: deleted report/label stable visual-property rename rejection should still select the deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1872: deleted report/label stable visual-property rename rejection should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1872: deleted report/label stable visual-property rename rejection should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1872: deleted report/label stable visual-property rename rejection should preserve selected deleted-row containing-section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1872: deleted report/label stable visual-property rename rejection should expose containing detail-band metadata");
    };

    const auto run_deleted_report_property_rename_missing_selector = [&](const fs::path& asset_path,
                                                                         const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto missing_selector_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-rename",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--new-property-name", "DisplayExpr",
                "--unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(missing_selector_process.exit_code == 4,
               "#1872: deleted report/label stable visual-property rename missing selector should fail");
        expect_contains(missing_selector_process.stdout_text, "\"visualPropertyRename\": null",
                        "#1872: missing-selector deleted report/label stable visual-property rename JSON should not expose stale rename objects");
        expect_not_contains(missing_selector_process.stdout_text, "\"dryRun\": false",
                            "#2218: missing-selector deleted report/label stable visual-property rename JSON should not expose stale committed state");
        expect_not_contains(missing_selector_process.stdout_text, "\"mutatesAsset\": true",
                            "#2218: missing-selector deleted report/label stable visual-property rename JSON should not expose stale mutation state");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoAvailable\": true",
                            "#2163: missing-selector deleted report/label stable visual-property rename JSON should not advertise undo availability");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoLabel\":",
                            "#2208: missing-selector deleted report/label stable visual-property rename JSON should not expose stale undo labels");
        expect_contains(missing_selector_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1872: missing-selector deleted report/label stable visual-property rename JSON should report selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "middle-field-guid", "DisplayExpr").empty() &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1872: missing-selector deleted report/label stable visual-property rename should preserve DBF state");
        (void)label;
    };

    run_deleted_report_property_rename_rejection(temp_root / "deleted_report_property_rename.frx",
                                                 "deleted_report_property_rename.frx",
                                                 "report");
    run_deleted_report_property_rename_rejection(temp_root / "deleted_report_property_rename.lbx",
                                                 "deleted_report_property_rename.lbx",
                                                 "label");
    run_deleted_report_property_rename_missing_selector(temp_root / "deleted_report_property_rename_missing_selector.frx",
                                                        "report");
    run_deleted_report_property_rename_missing_selector(temp_root / "deleted_report_property_rename_missing_selector.lbx",
                                                        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_rejects_deleted_report_visual_property_rename_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_rename_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1865: deleted report/label rename-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_rename_batch_rejection = [&](const fs::path& asset_path,
                                                                        const std::string& title,
                                                                        const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rename_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-rename-batch",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--new-property-name", "DisplayExpr",
                "--unique-id", "middle-field-guid",
                "--property-name", "WIDTH",
                "--new-property-name", "DisplayWidth",
                "--unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        if (rename_batch_process.exit_code != 4) {
            std::cerr << "studio host " << label << " stable deleted report property rename-batch stdout:\n"
                      << rename_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property rename-batch stderr:\n"
                      << rename_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_batch_process.exit_code == 4,
               "#1865: deleted report/label stable visual-property rename-batch should reject direct FRX/LBX fields");
        expect_contains(rename_batch_process.stdout_text, "\"visualPropertyRenameBatch\": null",
                        "#1865: failed deleted report/label stable visual-property rename-batch JSON should not expose stale batch objects");
        expect_not_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
                            "#2219: failed deleted report/label stable visual-property rename-batch JSON should not expose stale committed state");
        expect_not_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
                            "#2219: failed deleted report/label stable visual-property rename-batch JSON should not expose stale mutation state");
        expect_not_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
                            "#2164: failed deleted report/label stable visual-property rename-batch JSON should not advertise undo availability");
        expect_not_contains(rename_batch_process.stdout_text, "\"undoLabel\":",
                            "#2209: failed deleted report/label stable visual-property rename-batch JSON should not expose stale undo labels");
        expect_contains(rename_batch_process.stdout_text, "Direct DBF-backed fields cannot be renamed per object.",
                        "#1865: failed deleted report/label stable visual-property rename-batch JSON should report direct-field errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "WIDTH") == "50" &&
                   visual_object_property(asset_path, "middle-field-guid", "DisplayExpr").empty() &&
                   visual_object_property(asset_path, "right-field-guid", "DisplayWidth").empty() &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1865: failed deleted report/label stable visual-property rename-batch should not mutate DBF-backed fields");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property rename-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property rename-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1865: deleted report/label stable visual-property rename-batch rejection reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1865: deleted report/label stable visual-property rename-batch rejection should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1865: deleted label stable visual-property rename-batch rejection should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2063: stable deleted report/label visual-property rename-batch rejection JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1865: deleted report/label stable visual-property rename-batch rejection should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1865: deleted report/label stable visual-property rename-batch rejection should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1865: deleted report/label stable visual-property rename-batch rejection should still select the deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1865: deleted report/label stable visual-property rename-batch rejection should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1865: deleted report/label stable visual-property rename-batch rejection should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1865: deleted report/label stable visual-property rename-batch rejection should preserve selected deleted-row containing-section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1865: deleted report/label stable visual-property rename-batch rejection should expose containing detail-band metadata");
    };

    const auto run_deleted_report_property_rename_batch_missing_selector = [&](const fs::path& asset_path,
                                                                               const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto missing_selector_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-rename-batch",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--new-property-name", "DisplayExpr",
                "--unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(missing_selector_process.exit_code == 4,
               "#1865: deleted report/label stable visual-property rename-batch missing selector should fail");
        expect_contains(missing_selector_process.stdout_text, "\"visualPropertyRenameBatch\": null",
                        "#1865: missing-selector deleted report/label stable visual-property rename-batch JSON should not expose stale batch objects");
        expect_not_contains(missing_selector_process.stdout_text, "\"dryRun\": false",
                            "#2219: missing-selector deleted report/label stable visual-property rename-batch JSON should not expose stale committed state");
        expect_not_contains(missing_selector_process.stdout_text, "\"mutatesAsset\": true",
                            "#2219: missing-selector deleted report/label stable visual-property rename-batch JSON should not expose stale mutation state");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoAvailable\": true",
                            "#2164: missing-selector deleted report/label stable visual-property rename-batch JSON should not advertise undo availability");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoLabel\":",
                            "#2209: missing-selector deleted report/label stable visual-property rename-batch JSON should not expose stale undo labels");
        expect_contains(missing_selector_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1865: missing-selector deleted report/label stable visual-property rename-batch JSON should report selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.value" &&
                   visual_object_property(asset_path, "middle-field-guid", "DisplayExpr").empty() &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1865: missing-selector deleted report/label stable visual-property rename-batch should preserve DBF state");
        (void)label;
    };

    run_deleted_report_property_rename_batch_rejection(temp_root / "deleted_report_property_rename_batch.frx",
                                                       "deleted_report_property_rename_batch.frx",
                                                       "report");
    run_deleted_report_property_rename_batch_rejection(temp_root / "deleted_report_property_rename_batch.lbx",
                                                       "deleted_report_property_rename_batch.lbx",
                                                       "label");
    run_deleted_report_property_rename_batch_missing_selector(temp_root / "deleted_report_property_rename_batch_missing_selector.frx",
                                                              "report");
    run_deleted_report_property_rename_batch_missing_selector(temp_root / "deleted_report_property_rename_batch_missing_selector.lbx",
                                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_rejects_deleted_report_visual_property_reorder_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_reorder_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1873: deleted report/label reorder fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_reorder_rejection = [&](const fs::path& asset_path,
                                                                   const std::string& title,
                                                                   const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto reorder_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-reorder",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--placement", "first",
                "--unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_process.exit_code != 4) {
            std::cerr << "studio host " << label << " stable deleted report property reorder stdout:\n"
                      << reorder_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property reorder stderr:\n"
                      << reorder_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_process.exit_code == 4,
               "#1873: deleted report/label stable visual-property reorder should reject direct FRX/LBX fields");
        expect_contains(reorder_process.stdout_text, "\"visualPropertyReorder\": null",
                        "#1873: failed deleted report/label stable visual-property reorder JSON should not expose stale reorder objects");
        expect_not_contains(reorder_process.stdout_text, "\"dryRun\": false",
                            "#2220: failed deleted report/label stable visual-property reorder JSON should not expose stale committed state");
        expect_not_contains(reorder_process.stdout_text, "\"mutatesAsset\": true",
                            "#2220: failed deleted report/label stable visual-property reorder JSON should not expose stale mutation state");
        expect_not_contains(reorder_process.stdout_text, "\"undoAvailable\": true",
                            "#2165: failed deleted report/label stable visual-property reorder JSON should not advertise undo availability");
        expect_not_contains(reorder_process.stdout_text, "\"undoLabel\":",
                            "#2210: failed deleted report/label stable visual-property reorder JSON should not expose stale undo labels");
        expect_contains(reorder_process.stdout_text, "Direct DBF-backed fields cannot be reordered per object.",
                        "#1873: failed deleted report/label stable visual-property reorder JSON should report direct-field errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1873: failed deleted report/label stable visual-property reorder should preserve DBF-backed fields");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property reorder reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property reorder reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1873: deleted report/label stable visual-property reorder rejection reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1873: deleted report/label stable visual-property reorder rejection should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1873: deleted label stable visual-property reorder rejection should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2064: stable deleted report/label visual-property reorder rejection JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1873: deleted report/label stable visual-property reorder rejection should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1873: deleted report/label stable visual-property reorder rejection should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1873: deleted report/label stable visual-property reorder rejection should still select the deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1873: deleted report/label stable visual-property reorder rejection should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1873: deleted report/label stable visual-property reorder rejection should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1873: deleted report/label stable visual-property reorder rejection should preserve selected deleted-row containing-section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1873: deleted report/label stable visual-property reorder rejection should expose containing detail-band metadata");
    };

    const auto run_deleted_report_property_reorder_missing_selector = [&](const fs::path& asset_path,
                                                                          const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto missing_selector_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-reorder",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--placement", "first",
                "--unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(missing_selector_process.exit_code == 4,
               "#1873: deleted report/label stable visual-property reorder missing selector should fail");
        expect_contains(missing_selector_process.stdout_text, "\"visualPropertyReorder\": null",
                        "#1873: missing-selector deleted report/label stable visual-property reorder JSON should not expose stale reorder objects");
        expect_not_contains(missing_selector_process.stdout_text, "\"dryRun\": false",
                            "#2220: missing-selector deleted report/label stable visual-property reorder JSON should not expose stale committed state");
        expect_not_contains(missing_selector_process.stdout_text, "\"mutatesAsset\": true",
                            "#2220: missing-selector deleted report/label stable visual-property reorder JSON should not expose stale mutation state");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoAvailable\": true",
                            "#2165: missing-selector deleted report/label stable visual-property reorder JSON should not advertise undo availability");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoLabel\":",
                            "#2210: missing-selector deleted report/label stable visual-property reorder JSON should not expose stale undo labels");
        expect_contains(missing_selector_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1873: missing-selector deleted report/label stable visual-property reorder JSON should report selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1873: missing-selector deleted report/label stable visual-property reorder should preserve DBF state");
        (void)label;
    };

    run_deleted_report_property_reorder_rejection(temp_root / "deleted_report_property_reorder.frx",
                                                  "deleted_report_property_reorder.frx",
                                                  "report");
    run_deleted_report_property_reorder_rejection(temp_root / "deleted_report_property_reorder.lbx",
                                                  "deleted_report_property_reorder.lbx",
                                                  "label");
    run_deleted_report_property_reorder_missing_selector(temp_root / "deleted_report_property_reorder_missing_selector.frx",
                                                         "report");
    run_deleted_report_property_reorder_missing_selector(temp_root / "deleted_report_property_reorder_missing_selector.lbx",
                                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_rejects_deleted_report_visual_property_reorder_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_reorder_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_deleted = [](const fs::path& asset_path, const std::string& unique_id) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, unique_id),
               "#1866: deleted report/label reorder-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_reorder_batch_rejection = [&](const fs::path& asset_path,
                                                                         const std::string& title,
                                                                         const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto reorder_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-reorder-batch",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--placement", "first",
                "--unique-id", "middle-field-guid",
                "--property-name", "WIDTH",
                "--placement", "last",
                "--unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_batch_process.exit_code != 4) {
            std::cerr << "studio host " << label << " stable deleted report property reorder-batch stdout:\n"
                      << reorder_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property reorder-batch stderr:\n"
                      << reorder_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_batch_process.exit_code == 4,
               "#1866: deleted report/label stable visual-property reorder-batch should reject direct FRX/LBX fields");
        expect_contains(reorder_batch_process.stdout_text, "\"visualPropertyReorderBatch\": null",
                        "#1866: failed deleted report/label stable visual-property reorder-batch JSON should not expose stale batch objects");
        expect_not_contains(reorder_batch_process.stdout_text, "\"dryRun\": false",
                            "#2221: failed deleted report/label stable visual-property reorder-batch JSON should not expose stale committed state");
        expect_not_contains(reorder_batch_process.stdout_text, "\"mutatesAsset\": true",
                            "#2221: failed deleted report/label stable visual-property reorder-batch JSON should not expose stale mutation state");
        expect_not_contains(reorder_batch_process.stdout_text, "\"undoAvailable\": true",
                            "#2166: failed deleted report/label stable visual-property reorder-batch JSON should not advertise undo availability");
        expect_not_contains(reorder_batch_process.stdout_text, "\"undoLabel\":",
                            "#2211: failed deleted report/label stable visual-property reorder-batch JSON should not expose stale undo labels");
        expect_contains(reorder_batch_process.stdout_text, "Direct DBF-backed fields cannot be reordered per object.",
                        "#1866: failed deleted report/label stable visual-property reorder-batch JSON should report direct-field errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "WIDTH") == "50" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1866: failed deleted report/label stable visual-property reorder-batch should preserve DBF-backed fields");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property reorder-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property reorder-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1866: deleted report/label stable visual-property reorder-batch rejection reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1866: deleted report/label stable visual-property reorder-batch rejection should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1866: deleted label stable visual-property reorder-batch rejection should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2065: stable deleted report/label visual-property reorder-batch rejection JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1866: deleted report/label stable visual-property reorder-batch rejection should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1866: deleted report/label stable visual-property reorder-batch rejection should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1866: deleted report/label stable visual-property reorder-batch rejection should still select the deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1866: deleted report/label stable visual-property reorder-batch rejection should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1866: deleted report/label stable visual-property reorder-batch rejection should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 800",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1866: deleted report/label stable visual-property reorder-batch rejection should preserve selected deleted-row containing-section metadata");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 2"
            },
            "#1866: deleted report/label stable visual-property reorder-batch rejection should expose containing detail-band metadata");
    };

    const auto run_deleted_report_property_reorder_batch_missing_selector = [&](const fs::path& asset_path,
                                                                                const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto missing_selector_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-reorder-batch",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--placement", "first",
                "--unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(missing_selector_process.exit_code == 4,
               "#1866: deleted report/label stable visual-property reorder-batch missing selector should fail");
        expect_contains(missing_selector_process.stdout_text, "\"visualPropertyReorderBatch\": null",
                        "#1866: missing-selector deleted report/label stable visual-property reorder-batch JSON should not expose stale batch objects");
        expect_not_contains(missing_selector_process.stdout_text, "\"dryRun\": false",
                            "#2221: missing-selector deleted report/label stable visual-property reorder-batch JSON should not expose stale committed state");
        expect_not_contains(missing_selector_process.stdout_text, "\"mutatesAsset\": true",
                            "#2221: missing-selector deleted report/label stable visual-property reorder-batch JSON should not expose stale mutation state");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoAvailable\": true",
                            "#2166: missing-selector deleted report/label stable visual-property reorder-batch JSON should not advertise undo availability");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoLabel\":",
                            "#2211: missing-selector deleted report/label stable visual-property reorder-batch JSON should not expose stale undo labels");
        expect_contains(missing_selector_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1866: missing-selector deleted report/label stable visual-property reorder-batch JSON should report selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1866: missing-selector deleted report/label stable visual-property reorder-batch should preserve DBF state");
        (void)label;
    };

    run_deleted_report_property_reorder_batch_rejection(temp_root / "deleted_report_property_reorder_batch.frx",
                                                        "deleted_report_property_reorder_batch.frx",
                                                        "report");
    run_deleted_report_property_reorder_batch_rejection(temp_root / "deleted_report_property_reorder_batch.lbx",
                                                        "deleted_report_property_reorder_batch.lbx",
                                                        "label");
    run_deleted_report_property_reorder_batch_missing_selector(temp_root / "deleted_report_property_reorder_batch_missing_selector.frx",
                                                               "report");
    run_deleted_report_property_reorder_batch_missing_selector(temp_root / "deleted_report_property_reorder_batch_missing_selector.lbx",
                                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif
#endif
#endif

}  // namespace cf_test_studio_host_json
