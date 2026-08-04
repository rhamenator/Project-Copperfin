// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_copies_deleted_report_visual_properties_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_copy_stable_json_tests";
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
               "#1869: deleted report/label copy fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_copy = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto copy_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-copy",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        if (copy_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property copy stdout:\n"
                      << copy_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property copy stderr:\n"
                      << copy_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(copy_process.exit_code == 0,
               "#1869: deleted report/label stable visual-property copy JSON should exit successfully");
        expect_contains(copy_process.stdout_text, "\"visualPropertyCopy\": {",
                        "#1869: deleted report/label stable visual-property copy JSON should expose a copy object");
        expect_contains(copy_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#1869: deleted report/label stable visual-property copy JSON should expose affected property counts");
        expect_contains(copy_process.stdout_text, "\"dryRun\": false",
                        "#1869: deleted report/label stable visual-property copy JSON should expose committed state");
        expect_contains(copy_process.stdout_text, "\"mutatesAsset\": true",
                        "#1869: deleted report/label stable visual-property copy JSON should expose mutation state");
        expect_contains(copy_process.stdout_text, "\"undoAvailable\": true",
                        "#1869: deleted report/label stable visual-property copy JSON should expose undo availability");
        expect_contains(copy_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                        "#2159: deleted report/label stable visual-property copy JSON should expose copied-property undo labels");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "middle.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1869: deleted report/label stable visual-property copy should copy values without changing deleted state");

        const auto target_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (target_reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property copy target reopen stdout:\n"
                      << target_reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property copy target reopen stderr:\n"
                      << target_reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(target_reopen_process.exit_code == 0,
               "#1869: deleted report/label stable visual-property copy target reopen should exit successfully");
        expect_contains(target_reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1869: deleted report/label stable visual-property copy should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(target_reopen_process.stdout_text, "\"isLabel\": true",
                            "#1869: deleted label stable visual-property copy should retain label identity");
        }
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve live preview availability");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve live preview left bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve live preview top bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve live preview right bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve live preview bottom bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve live preview width");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve live preview height");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve deleted preview availability");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve deleted preview left bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve deleted preview top bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve deleted preview right bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve deleted preview bottom bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve deleted preview width");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2060: stable deleted report/label visual-property copy JSON should preserve deleted preview height");
        expect_contains(target_reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1869: deleted report/label stable visual-property copy should preserve live sibling counts");
        expect_contains(target_reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1869: deleted report/label stable visual-property copy should preserve deleted object counts");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1869: deleted report/label stable visual-property copy should select the copied target row");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1869: deleted report/label stable visual-property copy should preserve containing-section availability");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1869: deleted report/label stable visual-property copy should serialize containing-section metadata");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1869: deleted report/label stable visual-property copy should preserve report object selection kind");
        expect_contains(target_reopen_process.stdout_text, "\"recordIndex\": 4",
                        "#1869: deleted report/label stable visual-property copy should preserve target record indexes");
        expect_contains(target_reopen_process.stdout_text, "\"deleted\": true",
                        "#1869: deleted report/label stable visual-property copy should preserve target deleted state");
        expect_contains(target_reopen_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1869: deleted report/label stable visual-property copy should preserve target object kind");
        expect_contains(target_reopen_process.stdout_text, "\"expression\": \"middle.value\"",
                        "#1869: deleted report/label stable visual-property copy should refresh copied target expressions");
        expect_contains(target_reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#1869: deleted report/label stable visual-property copy should preserve target stable identities");
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
            "#1869: deleted report/label stable visual-property copy should refresh target deleted-row section metadata");
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
            "#1869: deleted report/label stable visual-property copy should expose containing detail-band metadata");

        const auto source_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);
        expect(source_reopen_process.exit_code == 0,
               "#1869: deleted report/label stable visual-property copy source reopen should exit successfully");
        expect_contains_in_order(
            source_reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1869: deleted report/label stable visual-property copy should preserve selected source expressions");

        write_synthetic_report_table_for_layout_distribution_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto geometry_copy_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-copy",
                "--path", asset_path.string(),
                "--property-name", "HPOS",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        if (geometry_copy_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry copy stdout:\n"
                      << geometry_copy_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry copy stderr:\n"
                      << geometry_copy_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_copy_process.exit_code == 0,
               "#2193: deleted report/label stable visual-property geometry copy JSON should exit successfully");
        expect_contains(geometry_copy_process.stdout_text, "\"visualPropertyCopy\": {",
                        "#2193: deleted report/label stable visual-property geometry copy JSON should expose a copy object");
        expect_contains(geometry_copy_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#2193: deleted report/label stable visual-property geometry copy JSON should expose affected property counts");
        expect_contains(geometry_copy_process.stdout_text, "\"dryRun\": false",
                        "#2193: deleted report/label stable visual-property geometry copy JSON should expose committed state");
        expect_contains(geometry_copy_process.stdout_text, "\"mutatesAsset\": true",
                        "#2193: deleted report/label stable visual-property geometry copy JSON should expose mutation state");
        expect_contains(geometry_copy_process.stdout_text, "\"undoAvailable\": true",
                        "#2193: deleted report/label stable visual-property geometry copy JSON should expose undo availability");
        expect_contains(geometry_copy_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2193: deleted report/label stable visual-property geometry copy JSON should expose geometry undo labels");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS") == "175" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "175" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#2193: deleted report/label stable visual-property geometry copy should copy geometry without changing deleted state");

        const auto geometry_target_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (geometry_target_reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry copy target reopen stdout:\n"
                      << geometry_target_reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry copy target reopen stderr:\n"
                      << geometry_target_reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_target_reopen_process.exit_code == 0,
               "#2193: deleted report/label stable visual-property geometry copy target reopen should exit successfully");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2193: deleted report/label stable visual-property geometry copy should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(geometry_target_reopen_process.stdout_text, "\"isLabel\": true",
                            "#2193: deleted label stable visual-property geometry copy should retain label identity");
        }
        expect_contains(geometry_target_reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2193: deleted report/label stable visual-property geometry copy should select the copied target row");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2193: deleted report/label stable visual-property geometry copy should preserve report object selection kind");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"recordIndex\": 4",
                        "#2193: deleted report/label stable visual-property geometry copy should preserve target record indexes");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"deleted\": true",
                        "#2193: deleted report/label stable visual-property geometry copy should preserve target deleted state");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"left\": 175",
                        "#2193: deleted report/label stable visual-property geometry copy should refresh copied target left metadata");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"right\": 225",
                        "#2193: deleted report/label stable visual-property geometry copy should refresh copied target bounds metadata");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#2193: deleted report/label stable visual-property geometry copy should preserve target stable identities");
    };

    const auto run_deleted_report_property_copy_failure = [&](const fs::path& asset_path,
                                                              const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto missing_selector_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-copy",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--source-unique-id", "missing-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        expect(missing_selector_process.exit_code == 4,
               "#1869: deleted report/label stable visual-property copy missing selector should fail");
        expect_contains(missing_selector_process.stdout_text, "\"visualPropertyCopy\": null",
                        "#1869: failed deleted report/label stable visual-property copy JSON should not expose stale copy objects");
        expect_not_contains(missing_selector_process.stdout_text, "\"dryRun\": false",
                            "#2214: failed deleted report/label stable visual-property copy JSON should not expose stale committed state");
        expect_not_contains(missing_selector_process.stdout_text, "\"mutatesAsset\": true",
                            "#2214: failed deleted report/label stable visual-property copy JSON should not expose stale mutation state");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoAvailable\": true",
                            "#2188: failed deleted report/label stable visual-property copy JSON should not advertise undo availability");
        expect_not_contains(missing_selector_process.stdout_text, "\"undoLabel\":",
                            "#2204: failed deleted report/label stable visual-property copy JSON should not expose stale undo labels");
        expect_contains(missing_selector_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1869: failed deleted report/label stable visual-property copy JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1869: failed deleted report/label stable visual-property copy should preserve DBF state");
        (void)label;
    };

    run_deleted_report_property_copy(temp_root / "deleted_report_property_copy.frx",
                                     "deleted_report_property_copy.frx",
                                     "report");
    run_deleted_report_property_copy(temp_root / "deleted_report_property_copy.lbx",
                                     "deleted_report_property_copy.lbx",
                                     "label");
    run_deleted_report_property_copy_failure(temp_root / "deleted_report_property_copy_missing_selector.frx",
                                             "report");
    run_deleted_report_property_copy_failure(temp_root / "deleted_report_property_copy_missing_selector.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_copies_deleted_report_visual_property_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_property_copy_batch_stable_json_tests";
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
               "#1868: deleted report/label copy-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_property_copy_batch = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto copy_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-copy-batch",
                "--path", asset_path.string(),
                "--property-name", "EXPR",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        if (copy_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property copy-batch stdout:\n"
                      << copy_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property copy-batch stderr:\n"
                      << copy_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(copy_batch_process.exit_code == 0,
               "#1868: deleted report/label stable visual-property copy-batch JSON should exit successfully");
        expect_contains(copy_batch_process.stdout_text, "\"visualPropertyCopyBatch\": {",
                        "#1868: deleted report/label stable visual-property copy-batch JSON should expose a batch object");
        expect_contains(copy_batch_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#1868: deleted report/label stable visual-property copy-batch JSON should expose affected property counts");
        expect_contains(copy_batch_process.stdout_text, "\"dryRun\": false",
                        "#1868: deleted report/label stable visual-property copy-batch JSON should expose committed state");
        expect_contains(copy_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1868: deleted report/label stable visual-property copy-batch JSON should expose mutation state");
        expect_contains(copy_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1868: deleted report/label stable visual-property copy-batch JSON should expose undo availability");
        expect_contains(copy_batch_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                        "#2160: deleted report/label stable visual-property copy-batch JSON should expose copied-property undo labels");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "middle.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1868: deleted report/label stable visual-property copy-batch should copy values without changing deleted state");

        const auto target_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (target_reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report property copy-batch target reopen stdout:\n"
                      << target_reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report property copy-batch target reopen stderr:\n"
                      << target_reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(target_reopen_process.exit_code == 0,
               "#1868: deleted report/label stable visual-property copy-batch target reopen should exit successfully");
        expect_contains(target_reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1868: deleted report/label stable visual-property copy-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(target_reopen_process.stdout_text, "\"isLabel\": true",
                            "#1868: deleted label stable visual-property copy-batch should retain label identity");
        }
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve live preview availability");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve live preview left bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve live preview top bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve live preview right bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve live preview bottom bounds");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve live preview width");
        expect_contains(target_reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve live preview height");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve deleted preview availability");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve deleted preview left bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve deleted preview top bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve deleted preview right bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve deleted preview width");
        expect_contains(target_reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2057: stable deleted report/label visual-property copy-batch JSON should preserve deleted preview height");
        expect_contains(target_reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1868: deleted report/label stable visual-property copy-batch should preserve live sibling counts");
        expect_contains(target_reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1868: deleted report/label stable visual-property copy-batch should preserve deleted object counts");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1868: deleted report/label stable visual-property copy-batch should select the copied target row");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1868: deleted report/label stable visual-property copy-batch should preserve containing-section availability");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1868: deleted report/label stable visual-property copy-batch should serialize containing-section metadata");
        expect_contains(target_reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1868: deleted report/label stable visual-property copy-batch should preserve report object selection kind");
        expect_contains(target_reopen_process.stdout_text, "\"recordIndex\": 4",
                        "#1868: deleted report/label stable visual-property copy-batch should preserve target record indexes");
        expect_contains(target_reopen_process.stdout_text, "\"deleted\": true",
                        "#1868: deleted report/label stable visual-property copy-batch should preserve target deleted state");
        expect_contains(target_reopen_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1868: deleted report/label stable visual-property copy-batch should preserve target object kind");
        expect_contains(target_reopen_process.stdout_text, "\"expression\": \"middle.value\"",
                        "#1868: deleted report/label stable visual-property copy-batch should refresh copied target expressions");
        expect_contains(target_reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#1868: deleted report/label stable visual-property copy-batch should preserve target stable identities");
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
            "#1868: deleted report/label stable visual-property copy-batch should refresh target deleted-row section metadata");
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
            "#1868: deleted report/label stable visual-property copy-batch should expose containing detail-band metadata");

        const auto source_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);
        expect(source_reopen_process.exit_code == 0,
               "#1868: deleted report/label stable visual-property copy-batch source reopen should exit successfully");
        expect_contains_in_order(
            source_reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1868: deleted report/label stable visual-property copy-batch should preserve selected source expressions");

        write_synthetic_report_table_for_layout_distribution_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto geometry_copy_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-copy-batch",
                "--path", asset_path.string(),
                "--property-name", "HPOS",
                "--source-unique-id", "middle-field-guid",
                "--target-unique-id", "right-field-guid",
                "--replace-existing", "true",
                "--json"
            },
            temp_root);

        if (geometry_copy_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry copy-batch stdout:\n"
                      << geometry_copy_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry copy-batch stderr:\n"
                      << geometry_copy_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_copy_batch_process.exit_code == 0,
               "#2195: deleted report/label stable visual-property geometry copy-batch JSON should exit successfully");
        expect_contains(geometry_copy_batch_process.stdout_text, "\"visualPropertyCopyBatch\": {",
                        "#2195: deleted report/label stable visual-property geometry copy-batch JSON should expose a batch object");
        expect_contains(geometry_copy_batch_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#2195: deleted report/label stable visual-property geometry copy-batch JSON should expose affected property counts");
        expect_contains(geometry_copy_batch_process.stdout_text, "\"dryRun\": false",
                        "#2195: deleted report/label stable visual-property geometry copy-batch JSON should expose committed state");
        expect_contains(geometry_copy_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#2195: deleted report/label stable visual-property geometry copy-batch JSON should expose mutation state");
        expect_contains(geometry_copy_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#2195: deleted report/label stable visual-property geometry copy-batch JSON should expose undo availability");
        expect_contains(geometry_copy_batch_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2195: deleted report/label stable visual-property geometry copy-batch JSON should expose geometry undo labels");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS") == "175" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "175" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#2195: deleted report/label stable visual-property geometry copy-batch should copy geometry without changing deleted state");

        const auto geometry_target_reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (geometry_target_reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report geometry copy-batch target reopen stdout:\n"
                      << geometry_target_reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report geometry copy-batch target reopen stderr:\n"
                      << geometry_target_reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(geometry_target_reopen_process.exit_code == 0,
               "#2195: deleted report/label stable visual-property geometry copy-batch target reopen should exit successfully");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2195: deleted report/label stable visual-property geometry copy-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(geometry_target_reopen_process.stdout_text, "\"isLabel\": true",
                            "#2195: deleted label stable visual-property geometry copy-batch should retain label identity");
        }
        expect_contains(geometry_target_reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2195: deleted report/label stable visual-property geometry copy-batch should select the copied target row");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2195: deleted report/label stable visual-property geometry copy-batch should preserve report object selection kind");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"recordIndex\": 4",
                        "#2195: deleted report/label stable visual-property geometry copy-batch should preserve target record indexes");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"deleted\": true",
                        "#2195: deleted report/label stable visual-property geometry copy-batch should preserve target deleted state");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"left\": 175",
                        "#2195: deleted report/label stable visual-property geometry copy-batch should refresh copied target left metadata");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"right\": 225",
                        "#2195: deleted report/label stable visual-property geometry copy-batch should refresh copied target bounds metadata");
        expect_contains(geometry_target_reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#2195: deleted report/label stable visual-property geometry copy-batch should preserve target stable identities");
    };

    const auto run_deleted_report_property_copy_batch_rollback = [&](const fs::path& asset_path,
                                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-copy-batch",
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
               "#1868: deleted report/label stable visual-property copy-batch missing selector should fail");
        expect_contains(rollback_process.stdout_text, "\"visualPropertyCopyBatch\": null",
                        "#1868: failed deleted report/label stable visual-property copy-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2215: failed deleted report/label stable visual-property copy-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2215: failed deleted report/label stable visual-property copy-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2189: failed deleted report/label stable visual-property copy-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2205: failed deleted report/label stable visual-property copy-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1868: failed deleted report/label stable visual-property copy-batch JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.value" &&
                   visual_object_property(asset_path, "left-field-guid", "EXPR") == "left.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1868: failed deleted report/label stable visual-property copy-batch should roll back earlier copies");

        write_synthetic_report_table_for_layout_distribution_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto geometry_rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-property-copy-batch",
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
               "#2197: deleted report/label stable visual-property geometry copy-batch missing selector should fail");
        expect_contains(geometry_rollback_process.stdout_text, "\"visualPropertyCopyBatch\": null",
                        "#2197: failed deleted report/label stable visual-property geometry copy-batch JSON should not expose stale batch objects");
        expect_not_contains(geometry_rollback_process.stdout_text, "\"dryRun\": false",
                            "#2215: failed deleted report/label stable visual-property geometry copy-batch JSON should not expose stale committed state");
        expect_not_contains(geometry_rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2215: failed deleted report/label stable visual-property geometry copy-batch JSON should not expose stale mutation state");
        expect_not_contains(geometry_rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2197: failed deleted report/label stable visual-property geometry copy-batch JSON should not advertise undo availability");
        expect_not_contains(geometry_rollback_process.stdout_text, "\"undoLabel\":",
                            "#2205: failed deleted report/label stable visual-property geometry copy-batch JSON should not expose stale undo labels");
        expect_contains(geometry_rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#2197: failed deleted report/label stable visual-property geometry copy-batch JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "HPOS") == "175" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "700" &&
                   visual_object_property(asset_path, "left-field-guid", "HPOS") == "100" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#2197: failed deleted report/label stable visual-property geometry copy-batch should roll back earlier geometry copies");
        (void)label;
    };

    run_deleted_report_property_copy_batch(temp_root / "deleted_report_property_copy_batch.frx",
                                           "deleted_report_property_copy_batch.frx",
                                           "report");
    run_deleted_report_property_copy_batch(temp_root / "deleted_report_property_copy_batch.lbx",
                                           "deleted_report_property_copy_batch.lbx",
                                           "label");
    run_deleted_report_property_copy_batch_rollback(temp_root / "deleted_report_property_copy_batch_rollback.frx",
                                                    "report");
    run_deleted_report_property_copy_batch_rollback(temp_root / "deleted_report_property_copy_batch_rollback.lbx",
                                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
