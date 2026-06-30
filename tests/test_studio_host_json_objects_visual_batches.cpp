#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_REPORT_VISUAL_SUBTREE_DUPLICATE_SKIP_HOST_SMOKE)
void test_studio_host_json_duplicates_report_visual_object_subtrees_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_subtree_duplicate_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_subtree_duplicate = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto duplicate_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "MiddleFieldCopy",
                "--new-name", "MiddleFieldCopy",
                "--new-unique-id", "middle-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object subtree duplicate stdout:\n"
                      << duplicate_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object subtree duplicate stderr:\n"
                      << duplicate_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_process.exit_code == 0,
               "#1858: report/label stable visual-object duplicate-subtree JSON should exit successfully");
        expect_contains(duplicate_process.stdout_text, "\"visualObjectDuplicateSubtree\": {",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose a result object");
        expect_contains(duplicate_process.stdout_text, "\"rootRecordIndex\": 5",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose appended root indexes");
        expect_contains(duplicate_process.stdout_text, "\"copiedCount\": 1",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose copied counts");
        expect_contains(duplicate_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose affected object counts");
        expect_contains(duplicate_process.stdout_text, "\"rootObjectName\": \"MiddleFieldCopy\"",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose copied root object names");
        expect_contains(duplicate_process.stdout_text, "\"rootUniqueId\": \"middle-copy-guid\"",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose copied root unique ids");
        expect_contains(duplicate_process.stdout_text, "\"rootParentName\": \"\"",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose copied root parent names");
        expect_contains(duplicate_process.stdout_text, "\"dryRun\": false",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose committed state");
        expect_contains(duplicate_process.stdout_text, "\"mutatesAsset\": true",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose mutation state");
        expect_contains(duplicate_process.stdout_text, "\"undoAvailable\": false",
                        "#1858: report/label stable visual-object duplicate-subtree JSON should expose undo availability");
        expect_contains(duplicate_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2173: report/label stable visual-object duplicate-subtree JSON should expose empty undo labels");
        expect(visual_object_count(asset_path) == before_count + 1U &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "middle-copy-guid") &&
                   visual_object_order(asset_path) ==
                       "left-field-guid,middle-field-guid,right-field-guid,middle-copy-guid",
               "#1858: report/label stable visual-object duplicate-subtree should append one copied flat layout row");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-copy-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object subtree duplicate reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object subtree duplicate reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1858: report/label stable visual-object duplicate-subtree reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1858: report/label stable visual-object duplicate-subtree should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1858: label stable visual-object duplicate-subtree should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2046: stable report/label object subtree duplicate JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2046: stable report/label object subtree duplicate JSON should keep deleted preview unavailable");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2046: stable report/label object subtree duplicate JSON should preserve zero deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 4",
                        "#1858: report/label stable visual-object duplicate-subtree should refresh live object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1858: report/label stable visual-object duplicate-subtree should expose copied-object section metadata");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1858: report/label stable visual-object duplicate-subtree should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": false",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionObjectIndex\": 3",
                "\"sectionObjectCount\": 4",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-copy-guid\""
            },
            "#1858: report/label stable visual-object duplicate-subtree should refresh selected copied-object metadata");
    };

    const auto run_subtree_duplicate_collision = [&](const fs::path& asset_path,
                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto collision_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "RightField",
                "--new-name", "MiddleFieldCopy",
                "--new-unique-id", "middle-copy-guid",
                "--json"
            },
            temp_root);

        expect(collision_process.exit_code == 4,
               "#1858: report/label stable visual-object duplicate-subtree should reject replacement collisions");
        expect_contains(collision_process.stdout_text, "\"visualObjectDuplicateSubtree\": null",
                        "#1858: failed report/label stable visual-object duplicate-subtree JSON should not expose stale result objects");
        expect_not_contains(collision_process.stdout_text, "\"dryRun\": false",
                            "#2222: failed report/label stable visual-object duplicate-subtree JSON should not expose stale committed state");
        expect_not_contains(collision_process.stdout_text, "\"mutatesAsset\": true",
                            "#2222: failed report/label stable visual-object duplicate-subtree JSON should not expose stale mutation state");
        expect_not_contains(collision_process.stdout_text, "\"undoAvailable\": true",
                            "#2176: failed report/label stable visual-object duplicate-subtree JSON should not advertise undo availability");
        expect_not_contains(collision_process.stdout_text, "\"undoLabel\":",
                            "#2200: failed report/label stable visual-object duplicate-subtree JSON should not expose stale undo labels");
        expect_contains(collision_process.stdout_text, "The requested replacement identity already exists in the asset.",
                        "#1858: failed report/label stable visual-object duplicate-subtree JSON should report collision errors");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   !visual_object_exists(asset_path, "middle-copy-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1858: failed report/label stable visual-object duplicate-subtree should not mutate layout rows");
        (void)label;
    };

    const auto run_subtree_duplicate_missing_selector = [&](const fs::path& asset_path,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const auto missing_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "missing-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "MiddleFieldCopy",
                "--new-name", "MiddleFieldCopy",
                "--new-unique-id", "middle-copy-guid",
                "--json"
            },
            temp_root);

        expect(missing_process.exit_code == 4,
               "#1858: report/label stable visual-object duplicate-subtree should reject missing stable selectors");
        expect_contains(missing_process.stdout_text, "\"visualObjectDuplicateSubtree\": null",
                        "#1858: missing-selector report/label stable visual-object duplicate-subtree JSON should not expose stale result objects");
        expect_not_contains(missing_process.stdout_text, "\"dryRun\": false",
                            "#2222: missing-selector report/label stable visual-object duplicate-subtree JSON should not expose stale committed state");
        expect_not_contains(missing_process.stdout_text, "\"mutatesAsset\": true",
                            "#2222: missing-selector report/label stable visual-object duplicate-subtree JSON should not expose stale mutation state");
        expect_not_contains(missing_process.stdout_text, "\"undoAvailable\": true",
                            "#2176: missing-selector report/label stable visual-object duplicate-subtree JSON should not advertise undo availability");
        expect_not_contains(missing_process.stdout_text, "\"undoLabel\":",
                            "#2200: missing-selector report/label stable visual-object duplicate-subtree JSON should not expose stale undo labels");
        expect_contains(missing_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1858: missing-selector report/label stable visual-object duplicate-subtree JSON should report selector errors");
        expect(visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid" &&
                   !visual_object_exists(asset_path, "middle-copy-guid"),
               "#1858: missing-selector report/label stable visual-object duplicate-subtree should not mutate layout rows");
        (void)label;
    };

    run_subtree_duplicate(temp_root / "object_subtree_duplicate.frx",
                          "object_subtree_duplicate.frx",
                          "report");
    run_subtree_duplicate(temp_root / "object_subtree_duplicate.lbx",
                          "object_subtree_duplicate.lbx",
                          "label");
    run_subtree_duplicate_collision(temp_root / "object_subtree_duplicate_collision.frx",
                                    "report");
    run_subtree_duplicate_collision(temp_root / "object_subtree_duplicate_collision.lbx",
                                    "label");
    run_subtree_duplicate_missing_selector(temp_root / "object_subtree_duplicate_missing.frx",
                                           "report");
    run_subtree_duplicate_missing_selector(temp_root / "object_subtree_duplicate_missing.lbx",
                                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_deleted_report_visual_object_subtrees_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_subtree_duplicate_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto mark_middle_deleted = [](const fs::path& asset_path) {
        const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "middle-field-guid",
            .deleted = true
        });
        expect(delete_result.ok && visual_object_deleted(asset_path, "middle-field-guid"),
               "#1859: deleted report/label duplicate-subtree fixture should start with a deleted root row");
    };

    const auto run_deleted_subtree_duplicate = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        mark_middle_deleted(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto duplicate_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "MiddleDeletedCopy",
                "--new-name", "MiddleDeletedCopy",
                "--new-unique-id", "middle-deleted-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object subtree duplicate stdout:\n"
                      << duplicate_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object subtree duplicate stderr:\n"
                      << duplicate_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_process.exit_code == 0,
               "#1859: deleted report/label stable visual-object duplicate-subtree JSON should exit successfully");
        expect_contains(duplicate_process.stdout_text, "\"visualObjectDuplicateSubtree\": {",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose a result object");
        expect_contains(duplicate_process.stdout_text, "\"rootRecordIndex\": 5",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose appended root indexes");
        expect_contains(duplicate_process.stdout_text, "\"copiedCount\": 1",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose copied counts");
        expect_contains(duplicate_process.stdout_text, "\"affectedObjectCount\": 1",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose affected object counts");
        expect_contains(duplicate_process.stdout_text, "\"rootObjectName\": \"MiddleDeletedCopy\"",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose copied root object names");
        expect_contains(duplicate_process.stdout_text, "\"rootUniqueId\": \"middle-deleted-copy-guid\"",
                        "#1859: deleted report/label stable visual-object duplicate-subtree JSON should expose copied root unique ids");
        expect_contains(duplicate_process.stdout_text, "\"rootParentName\": \"\"",
                        "#2175: deleted report/label stable visual-object duplicate-subtree JSON should expose copied root parent names");
        expect_contains(duplicate_process.stdout_text, "\"dryRun\": false",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose committed state");
        expect_contains(duplicate_process.stdout_text, "\"mutatesAsset\": true",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose mutation state");
        expect_contains(duplicate_process.stdout_text, "\"undoAvailable\": false",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose undo availability");
        expect_contains(duplicate_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2174: deleted report/label stable visual-object duplicate-subtree JSON should expose empty undo labels");
        expect(visual_object_count(asset_path) == before_count + 1U &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "middle-deleted-copy-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid") &&
                   visual_object_order(asset_path) ==
                       "left-field-guid,middle-field-guid,right-field-guid,middle-deleted-copy-guid",
               "#1859: deleted report/label stable visual-object duplicate-subtree should append a deleted copied row");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-deleted-copy-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object subtree duplicate reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object subtree duplicate reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1859: deleted report/label stable visual-object duplicate-subtree reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1859: deleted label stable visual-object duplicate-subtree should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2047: stable deleted report/label object subtree duplicate JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2047: stable deleted report/label object subtree duplicate JSON should expose deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 2",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should expose original and copied deleted rows");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should leave the report section selection empty");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSection\": null",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should serialize a null report section selection");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should select the copied deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should preserve containing-section availability");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1859: deleted report/label stable visual-object duplicate-subtree should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"field\"",
                "\"title\": \"MiddleDeletedCopy\"",
                "\"expression\": \"middle.value\"",
                "\"highlightCount\": 1"
            },
            "#1859: deleted report/label stable visual-object duplicate-subtree should preserve copied deleted-object metadata");
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
                "\"objectCount\": 2",
                "\"deletedObjectCount\": 2"
            },
            "#1859: deleted report/label stable visual-object duplicate-subtree should expose containing detail-band metadata");
    };

    const auto run_deleted_subtree_duplicate_collision = [&](const fs::path& asset_path,
                                                             const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        mark_middle_deleted(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);

        const auto collision_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-subtree",
                "--path", asset_path.string(),
                "--unique-id", "middle-field-guid",
                "--replacement-source-unique-id", "middle-field-guid",
                "--new-object-name", "MiddleDeletedCopy",
                "--new-name", "MiddleDeletedCopy",
                "--new-unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        expect(collision_process.exit_code == 4,
               "#1859: deleted report/label stable visual-object duplicate-subtree should reject replacement collisions");
        expect_contains(collision_process.stdout_text, "\"visualObjectDuplicateSubtree\": null",
                        "#1859: failed deleted report/label stable visual-object duplicate-subtree JSON should not expose stale result objects");
        expect_not_contains(collision_process.stdout_text, "\"dryRun\": false",
                            "#2223: failed deleted report/label stable visual-object duplicate-subtree JSON should not expose stale committed state");
        expect_not_contains(collision_process.stdout_text, "\"mutatesAsset\": true",
                            "#2223: failed deleted report/label stable visual-object duplicate-subtree JSON should not expose stale mutation state");
        expect_not_contains(collision_process.stdout_text, "\"undoAvailable\": true",
                            "#2177: failed deleted report/label stable visual-object duplicate-subtree JSON should not advertise undo availability");
        expect_not_contains(collision_process.stdout_text, "\"undoLabel\":",
                            "#2200: failed deleted report/label stable visual-object duplicate-subtree JSON should not expose stale undo labels");
        expect_contains(collision_process.stdout_text, "The requested replacement identity already exists in the asset.",
                        "#1859: failed deleted report/label stable visual-object duplicate-subtree JSON should report collision errors");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_exists(asset_path, "middle-deleted-copy-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1859: failed deleted report/label stable visual-object duplicate-subtree should not mutate layout rows");
        (void)label;
    };

    run_deleted_subtree_duplicate(temp_root / "deleted_object_subtree_duplicate.frx",
                                  "deleted_object_subtree_duplicate.frx",
                                  "report");
    run_deleted_subtree_duplicate(temp_root / "deleted_object_subtree_duplicate.lbx",
                                  "deleted_object_subtree_duplicate.lbx",
                                  "label");
    run_deleted_subtree_duplicate_collision(temp_root / "deleted_object_subtree_duplicate_collision.frx",
                                            "report");
    run_deleted_subtree_duplicate_collision(temp_root / "deleted_object_subtree_duplicate_collision.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_REPORT_VISUAL_UPDATE_BATCH_SKIP_HOST_SMOKE)
void test_studio_host_json_updates_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_update_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_report_object_update_batch = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-update-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "field-guid",
                "--property-name", "EXPR",
                "--property-value", "customer.contact",
                "--property-name", "WIDTH",
                "--property-value", "4300",
                "--selected-unique-id", "label-guid",
                "--property-name", "EXPR",
                "--property-value", "\"Updated invoice\"",
                "--property-name", "HPOS",
                "--property-value", "720",
                "--json"
            },
            temp_root);

        if (update_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object update-batch stdout:\n"
                      << update_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object update-batch stderr:\n"
                      << update_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_batch_process.exit_code == 0,
               "#1842: report/label stable visual-object update-batch JSON should exit successfully");
        expect_contains(update_batch_process.stdout_text, "\"visualObjectUpdateBatch\": {",
                        "#1842: report/label stable visual-object update-batch JSON should expose a batch object");
        expect_contains(update_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1842: report/label stable visual-object update-batch JSON should expose affected object counts");
        expect_contains(update_batch_process.stdout_text, "\"dryRun\": false",
                        "#1842: report/label stable visual-object update-batch JSON should expose committed state");
        expect_contains(update_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1842: report/label stable visual-object update-batch JSON should expose mutation state");
        expect_contains(update_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1842: report/label stable visual-object update-batch JSON should expose undo availability");
        expect_contains(update_batch_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2156: report/label stable visual-object update-batch JSON should expose the latest undo label");
        expect(visual_object_property(asset_path, "field-guid", "EXPR") == "customer.contact" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "4300" &&
                   visual_object_property(asset_path, "label-guid", "EXPR") == "\"Updated invoice\"" &&
                   visual_object_property(asset_path, "label-guid", "HPOS") == "720",
               "#1842: report/label stable visual-object update-batch should persist direct and memo-backed layout properties");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "field-guid", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#1842: report/label stable visual-object update-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1842: report/label stable visual-object update-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1842: label stable visual-object update-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 5500",
                        "#2048: stable report/label live visual-object update-batch JSON should refresh live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 5500",
                        "#2048: stable report/label live visual-object update-batch JSON should refresh live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#2048: stable report/label live visual-object update-batch JSON should preserve deleted preview height");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"width\": 4300",
                "\"right\": 5500",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.contact\""
            },
            "#1842: report/label stable visual-object update-batch should refresh selected object metadata after reopen");
    };

    const auto run_report_object_update_batch_rollback = [&](const fs::path& asset_path,
                                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-update-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "field-guid",
                "--property-name", "EXPR",
                "--property-value", "should.rollback",
                "--property-name", "WIDTH",
                "--property-value", "4444",
                "--selected-unique-id", "label-guid",
                "--property-name", "HPOS",
                "--property-value", "111",
                "--selected-unique-id", "missing-guid",
                "--property-name", "EXPR",
                "--property-value", "missing",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1842: report/label stable visual-object update-batch missing selector should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectUpdateBatch\": null",
                        "#1842: failed report/label stable visual-object update-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2228: failed report/label stable visual-object update-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2228: failed report/label stable visual-object update-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2184: failed report/label stable visual-object update-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2201: failed report/label stable visual-object update-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1842: failed report/label stable visual-object update-batch JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "field-guid", "EXPR") == "customer.company" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "4000" &&
                   visual_object_property(asset_path, "label-guid", "HPOS") == "900",
               "#1842: failed report/label stable visual-object update-batch should roll back earlier layout property mutations");
        (void)label;
    };

    run_report_object_update_batch(temp_root / "report_object_update_batch.frx",
                                   "report_object_update_batch.frx",
                                   "report");
    run_report_object_update_batch(temp_root / "report_object_update_batch.lbx",
                                   "report_object_update_batch.lbx",
                                   "label");
    run_report_object_update_batch_rollback(temp_root / "report_object_update_batch_rollback.frx",
                                            "report");
    run_report_object_update_batch_rollback(temp_root / "report_object_update_batch_rollback.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_update_batch_stable_json_tests";
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
               "#1863: deleted report/label update-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_object_update_batch = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto update_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-update-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--property-name", "EXPR",
                "--property-value", "middle.updated",
                "--property-name", "WIDTH",
                "--property-value", "4300",
                "--selected-unique-id", "right-field-guid",
                "--property-name", "EXPR",
                "--property-value", "right.updated",
                "--property-name", "HPOS",
                "--property-value", "720",
                "--json"
            },
            temp_root);

        if (update_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object update-batch stdout:\n"
                      << update_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object update-batch stderr:\n"
                      << update_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_batch_process.exit_code == 0,
               "#1863: deleted report/label stable visual-object update-batch JSON should exit successfully");
        expect_contains(update_batch_process.stdout_text, "\"visualObjectUpdateBatch\": {",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose a batch object");
        expect_contains(update_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose affected object counts");
        expect_contains(update_batch_process.stdout_text, "\"dryRun\": false",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose committed state");
        expect_contains(update_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose mutation state");
        expect_contains(update_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1863: deleted report/label stable visual-object update-batch JSON should expose undo availability");
        expect_contains(update_batch_process.stdout_text, "\"undoLabel\": \"Property HPOS\"",
                        "#2156: deleted report/label stable visual-object update-batch JSON should expose the latest undo label");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.updated" &&
                   visual_object_property(asset_path, "middle-field-guid", "WIDTH") == "4300" &&
                   visual_object_property(asset_path, "right-field-guid", "EXPR") == "right.updated" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "720" &&
                   visual_object_property(asset_path, "left-field-guid", "EXPR") == "left.value" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1863: deleted report/label stable visual-object update-batch should persist properties without changing deleted state");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object update-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object update-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1863: deleted report/label stable visual-object update-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1863: deleted report/label stable visual-object update-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1863: deleted label stable visual-object update-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2049: stable deleted report/label visual-object update-batch JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 4400",
                        "#2049: stable deleted report/label visual-object update-batch JSON should refresh deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 4300",
                        "#2049: stable deleted report/label visual-object update-batch JSON should refresh deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2049: stable deleted report/label visual-object update-batch JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1863: deleted report/label stable visual-object update-batch should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1863: deleted report/label stable visual-object update-batch should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1863: deleted report/label stable visual-object update-batch should select the updated deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1863: deleted report/label stable visual-object update-batch should preserve containing-section availability");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1863: deleted report/label stable visual-object update-batch should serialize containing-section metadata");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1863: deleted report/label stable visual-object update-batch should preserve report object selection kind");
        expect_contains(reopen_process.stdout_text, "\"recordIndex\": 3",
                        "#1863: deleted report/label stable visual-object update-batch should preserve updated record indexes");
        expect_contains(reopen_process.stdout_text, "\"deleted\": true",
                        "#1863: deleted report/label stable visual-object update-batch should preserve updated deleted state");
        expect_contains(reopen_process.stdout_text, "\"width\": 4300",
                        "#1863: deleted report/label stable visual-object update-batch should refresh updated width metadata");
        expect_contains(reopen_process.stdout_text, "\"right\": 4400",
                        "#1863: deleted report/label stable visual-object update-batch should refresh updated bounds metadata");
        expect_contains(reopen_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1863: deleted report/label stable visual-object update-batch should preserve updated object kind");
        expect_contains(reopen_process.stdout_text, "\"expression\": \"middle.updated\"",
                        "#1863: deleted report/label stable visual-object update-batch should refresh updated expressions");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"middle-field-guid\"",
                        "#1863: deleted report/label stable visual-object update-batch should preserve updated stable identities");
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
                "\"expression\": \"middle.updated\"",
                "\"width\": 4300",
                "\"right\": 4400",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1863: deleted report/label stable visual-object update-batch should refresh selected deleted-row section metadata");
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
            "#1863: deleted report/label stable visual-object update-batch should expose containing detail-band metadata");
    };

    const auto run_deleted_report_object_update_batch_rollback = [&](const fs::path& asset_path,
                                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-update-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--property-name", "EXPR",
                "--property-value", "should.rollback",
                "--property-name", "WIDTH",
                "--property-value", "4444",
                "--selected-unique-id", "right-field-guid",
                "--property-name", "HPOS",
                "--property-value", "111",
                "--selected-unique-id", "missing-guid",
                "--property-name", "EXPR",
                "--property-value", "missing",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1863: deleted report/label stable visual-object update-batch missing selector should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectUpdateBatch\": null",
                        "#1863: failed deleted report/label stable visual-object update-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2229: failed deleted report/label stable visual-object update-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2229: failed deleted report/label stable visual-object update-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2185: failed deleted report/label stable visual-object update-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2201: failed deleted report/label stable visual-object update-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1863: failed deleted report/label stable visual-object update-batch JSON should report missing selector errors");
        expect(visual_object_property(asset_path, "middle-field-guid", "EXPR") == "middle.value" &&
                   visual_object_property(asset_path, "middle-field-guid", "WIDTH") == "50" &&
                   visual_object_property(asset_path, "right-field-guid", "HPOS") == "100" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1863: failed deleted report/label stable visual-object update-batch should roll back earlier property mutations");
        (void)label;
    };

    run_deleted_report_object_update_batch(temp_root / "deleted_report_object_update_batch.frx",
                                           "deleted_report_object_update_batch.frx",
                                           "report");
    run_deleted_report_object_update_batch(temp_root / "deleted_report_object_update_batch.lbx",
                                           "deleted_report_object_update_batch.lbx",
                                           "label");
    run_deleted_report_object_update_batch_rollback(temp_root / "deleted_report_object_update_batch_rollback.frx",
                                                    "report");
    run_deleted_report_object_update_batch_rollback(temp_root / "deleted_report_object_update_batch_rollback.lbx",
                                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_REPORT_VISUAL_RENAME_BATCH_SKIP_HOST_SMOKE)
void test_studio_host_json_renames_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_rename_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_report_object_rename_batch = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto rename_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-rename-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "left-field-guid",
                "--new-unique-id", "left-renamed-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "right-renamed-guid",
                "--json"
            },
            temp_root);

        if (rename_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object rename-batch stdout:\n"
                      << rename_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object rename-batch stderr:\n"
                      << rename_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_batch_process.exit_code == 0,
               "#1843: report/label stable visual-object rename-batch JSON should exit successfully");
        expect_contains(rename_batch_process.stdout_text, "\"visualObjectRenameBatch\": {",
                        "#1843: report/label stable visual-object rename-batch JSON should expose a batch object");
        expect_contains(rename_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1843: report/label stable visual-object rename-batch JSON should expose affected object counts");
        expect_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
                        "#1843: report/label stable visual-object rename-batch JSON should expose committed state");
        expect_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1843: report/label stable visual-object rename-batch JSON should expose mutation state");
        expect_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1843: report/label stable visual-object rename-batch JSON should expose undo availability");
        expect_contains(rename_batch_process.stdout_text, "\"undoLabel\": \"Property UNIQUEID\"",
                        "#2168: report/label stable visual-object rename-batch JSON should expose renamed-identity undo labels");
        expect(visual_object_count(asset_path) == before_count &&
                   !visual_object_exists(asset_path, "left-field-guid") &&
                   !visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "left-renamed-guid") &&
                   visual_object_exists(asset_path, "right-renamed-guid") &&
                   visual_object_order(asset_path) == "left-renamed-guid,middle-field-guid,right-renamed-guid",
               "#1843: report/label stable visual-object rename-batch should replace identities without changing object order");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "left-renamed-guid", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#1843: report/label stable visual-object rename-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1843: report/label stable visual-object rename-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1843: label stable visual-object rename-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2050: stable report/label live visual-object rename-batch JSON should not fabricate deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2050: stable report/label live visual-object rename-batch JSON should preserve zero deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"left-renamed-guid\"",
                        "#1843: report/label stable visual-object rename-batch should preserve selected object identity after reopen");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"sectionObjectIndex\": 0",
                "\"objectKind\": \"field\"",
                "\"expression\": \"left.value\""
            },
            "#1843: report/label stable visual-object rename-batch should refresh selected renamed object metadata after reopen");
    };

    const auto run_report_object_rename_batch_rollback = [&](const fs::path& asset_path,
                                                             const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-rename-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "left-field-guid",
                "--new-unique-id", "left-rollback-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1843: report/label stable visual-object rename-batch identity collision should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectRenameBatch\": null",
                        "#1843: failed report/label stable visual-object rename-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2230: failed report/label stable visual-object rename-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2230: failed report/label stable visual-object rename-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2178: failed report/label stable visual-object rename-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed report/label stable visual-object rename-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "The requested identity value already exists in the asset.",
                        "#1843: failed report/label stable visual-object rename-batch JSON should report collision errors");
        expect(visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   !visual_object_exists(asset_path, "left-rollback-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1843: failed report/label stable visual-object rename-batch should roll back earlier identity mutations");
        (void)label;
    };

    run_report_object_rename_batch(temp_root / "report_object_rename_batch.frx",
                                   "report_object_rename_batch.frx",
                                   "report");
    run_report_object_rename_batch(temp_root / "report_object_rename_batch.lbx",
                                   "report_object_rename_batch.lbx",
                                   "label");
    run_report_object_rename_batch_rollback(temp_root / "report_object_rename_batch_rollback.frx",
                                            "report");
    run_report_object_rename_batch_rollback(temp_root / "report_object_rename_batch_rollback.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_deleted_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_rename_batch_stable_json_tests";
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
               "#1862: deleted report/label rename-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_object_rename_batch = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");
        const std::size_t before_count = visual_object_count(asset_path);

        const auto rename_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-rename-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--new-unique-id", "middle-renamed-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "right-renamed-guid",
                "--json"
            },
            temp_root);

        if (rename_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object rename-batch stdout:\n"
                      << rename_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object rename-batch stderr:\n"
                      << rename_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(rename_batch_process.exit_code == 0,
               "#1862: deleted report/label stable visual-object rename-batch JSON should exit successfully");
        expect_contains(rename_batch_process.stdout_text, "\"visualObjectRenameBatch\": {",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose a batch object");
        expect_contains(rename_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose affected object counts");
        expect_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose committed state");
        expect_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose mutation state");
        expect_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
                        "#1862: deleted report/label stable visual-object rename-batch JSON should expose undo availability");
        expect_contains(rename_batch_process.stdout_text, "\"undoLabel\": \"Property UNIQUEID\"",
                        "#2167: deleted report/label stable visual-object rename-batch JSON should expose renamed-identity undo labels");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   !visual_object_exists(asset_path, "middle-field-guid") &&
                   !visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "middle-renamed-guid") &&
                   visual_object_exists(asset_path, "right-renamed-guid") &&
                   visual_object_deleted(asset_path, "middle-renamed-guid") &&
                   visual_object_deleted(asset_path, "right-renamed-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-renamed-guid,right-renamed-guid",
               "#1862: deleted report/label stable visual-object rename-batch should replace deleted-row identities without changing order");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-renamed-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object rename-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object rename-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1862: deleted report/label stable visual-object rename-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1862: deleted report/label stable visual-object rename-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1862: deleted label stable visual-object rename-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2051: stable deleted report/label visual-object rename-batch JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1862: deleted report/label stable visual-object rename-batch should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1862: deleted report/label stable visual-object rename-batch should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1862: deleted report/label stable visual-object rename-batch should select the renamed deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1862: deleted report/label stable visual-object rename-batch should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1862: deleted report/label stable visual-object rename-batch should preserve report object selection kind");
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
                "\"uniqueId\": \"middle-renamed-guid\""
            },
            "#1862: deleted report/label stable visual-object rename-batch should preserve selected deleted-row containing-section metadata");
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
            "#1862: deleted report/label stable visual-object rename-batch should expose containing detail-band metadata");
    };

    const auto run_deleted_report_object_rename_batch_rollback = [&](const fs::path& asset_path,
                                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-rename-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--new-unique-id", "mid-rb-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "left-field-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1862: deleted report/label stable visual-object rename-batch identity collision should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectRenameBatch\": null",
                        "#1862: failed deleted report/label stable visual-object rename-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2231: failed deleted report/label stable visual-object rename-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2231: failed deleted report/label stable visual-object rename-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2179: failed deleted report/label stable visual-object rename-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed deleted report/label stable visual-object rename-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "The requested identity value already exists in the asset.",
                        "#1862: failed deleted report/label stable visual-object rename-batch JSON should report collision errors");
        expect(visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   !visual_object_exists(asset_path, "mid-rb-guid") &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1862: failed deleted report/label stable visual-object rename-batch should roll back earlier identity mutations");
        (void)label;
    };

    run_deleted_report_object_rename_batch(temp_root / "deleted_report_object_rename_batch.frx",
                                           "deleted_report_object_rename_batch.frx",
                                           "report");
    run_deleted_report_object_rename_batch(temp_root / "deleted_report_object_rename_batch.lbx",
                                           "deleted_report_object_rename_batch.lbx",
                                           "label");
    run_deleted_report_object_rename_batch_rollback(temp_root / "deleted_report_object_rename_batch_rollback.frx",
                                                    "report");
    run_deleted_report_object_rename_batch_rollback(temp_root / "deleted_report_object_rename_batch_rollback.lbx",
                                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

void test_studio_host_json_duplicates_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_duplicate_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_report_object_duplicate_batch = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto duplicate_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "left-field-guid",
                "--new-unique-id", "left-copy-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "right-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object duplicate-batch stdout:\n"
                      << duplicate_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object duplicate-batch stderr:\n"
                      << duplicate_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_batch_process.exit_code == 0,
               "#1844: report/label stable visual-object duplicate-batch JSON should exit successfully");
        expect_contains(duplicate_batch_process.stdout_text, "\"visualObjectDuplicateBatch\": {",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose a batch object");
        expect_contains(duplicate_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose affected object counts");
        expect_contains(duplicate_batch_process.stdout_text, "\"dryRun\": false",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose committed state");
        expect_contains(duplicate_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose mutation state");
        expect_contains(duplicate_batch_process.stdout_text, "\"undoAvailable\": false",
                        "#1844: report/label stable visual-object duplicate-batch JSON should expose undo availability");
        expect_contains(duplicate_batch_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2169: report/label stable visual-object duplicate-batch JSON should expose empty undo labels");
        expect(visual_object_count(asset_path) == before_count + 2U &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "left-copy-guid") &&
                   visual_object_exists(asset_path, "right-copy-guid") &&
                   visual_object_order(asset_path) ==
                       "left-field-guid,middle-field-guid,right-field-guid,left-copy-guid,right-copy-guid",
               "#1844: report/label stable visual-object duplicate-batch should append duplicates after original objects");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "left-copy-guid", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#1844: report/label stable visual-object duplicate-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1844: report/label stable visual-object duplicate-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1844: label stable visual-object duplicate-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should not fabricate deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2052: stable report/label live visual-object duplicate-batch JSON should preserve zero deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"left-copy-guid\"",
                        "#1844: report/label stable visual-object duplicate-batch should preserve selected duplicate identity after reopen");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"sectionObjectIndex\": 3",
                "\"sectionObjectCount\": 5",
                "\"objectKind\": \"field\"",
                "\"expression\": \"left.value\""
            },
            "#1844: report/label stable visual-object duplicate-batch should refresh selected duplicate metadata after reopen");
    };

    const auto run_report_object_duplicate_batch_rollback = [&](const fs::path& asset_path,
                                                                const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const std::size_t before_count = visual_object_count(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "left-field-guid",
                "--new-unique-id", "left-rollback-copy-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "left-rollback-copy-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1844: report/label stable visual-object duplicate-batch identity collision should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectDuplicateBatch\": null",
                        "#1844: failed report/label stable visual-object duplicate-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2224: failed report/label stable visual-object duplicate-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2224: failed report/label stable visual-object duplicate-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2180: failed report/label stable visual-object duplicate-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed report/label stable visual-object duplicate-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "The requested replacement identity already exists in the asset.",
                        "#1844: failed report/label stable visual-object duplicate-batch JSON should report collision errors");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_exists(asset_path, "left-field-guid") &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   !visual_object_exists(asset_path, "left-rollback-copy-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1844: failed report/label stable visual-object duplicate-batch should roll back earlier duplicates");
        (void)label;
    };

    run_report_object_duplicate_batch(temp_root / "report_object_duplicate_batch.frx",
                                      "report_object_duplicate_batch.frx",
                                      "report");
    run_report_object_duplicate_batch(temp_root / "report_object_duplicate_batch.lbx",
                                      "report_object_duplicate_batch.lbx",
                                      "label");
    run_report_object_duplicate_batch_rollback(temp_root / "report_object_duplicate_batch_rollback.frx",
                                               "report");
    run_report_object_duplicate_batch_rollback(temp_root / "report_object_duplicate_batch_rollback.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_deleted_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_duplicate_batch_stable_json_tests";
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
               "#1860: deleted report/label duplicate-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_object_duplicate_batch = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");
        const std::size_t before_count = visual_object_count(asset_path);

        const auto duplicate_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--new-unique-id", "middle-deleted-copy-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "right-deleted-copy-guid",
                "--json"
            },
            temp_root);

        if (duplicate_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object duplicate-batch stdout:\n"
                      << duplicate_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object duplicate-batch stderr:\n"
                      << duplicate_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(duplicate_batch_process.exit_code == 0,
               "#1860: deleted report/label stable visual-object duplicate-batch JSON should exit successfully");
        expect_contains(duplicate_batch_process.stdout_text, "\"visualObjectDuplicateBatch\": {",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose a batch object");
        expect_contains(duplicate_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose affected object counts");
        expect_contains(duplicate_batch_process.stdout_text, "\"dryRun\": false",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose committed state");
        expect_contains(duplicate_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose mutation state");
        expect_contains(duplicate_batch_process.stdout_text, "\"undoAvailable\": false",
                        "#1860: deleted report/label stable visual-object duplicate-batch JSON should expose undo availability");
        expect_contains(duplicate_batch_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2170: deleted report/label stable visual-object duplicate-batch JSON should expose empty undo labels");
        expect(visual_object_count(asset_path) == before_count + 2U &&
                   visual_object_exists(asset_path, "middle-field-guid") &&
                   visual_object_exists(asset_path, "right-field-guid") &&
                   visual_object_exists(asset_path, "middle-deleted-copy-guid") &&
                   visual_object_exists(asset_path, "right-deleted-copy-guid") &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   visual_object_deleted(asset_path, "middle-deleted-copy-guid") &&
                   visual_object_deleted(asset_path, "right-deleted-copy-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   visual_object_order(asset_path) ==
                       "left-field-guid,middle-field-guid,right-field-guid,middle-deleted-copy-guid,right-deleted-copy-guid",
               "#1860: deleted report/label stable visual-object duplicate-batch should append deleted duplicate rows");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "middle-deleted-copy-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object duplicate-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object duplicate-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1860: deleted report/label stable visual-object duplicate-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1860: deleted report/label stable visual-object duplicate-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1860: deleted label stable visual-object duplicate-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2053: stable deleted report/label visual-object duplicate-batch JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1860: deleted report/label stable visual-object duplicate-batch should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 4",
                        "#1860: deleted report/label stable visual-object duplicate-batch should expose original and copied deleted rows");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1860: deleted report/label stable visual-object duplicate-batch should select the copied deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1860: deleted report/label stable visual-object duplicate-batch should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1860: deleted report/label stable visual-object duplicate-batch should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-deleted-copy-guid\""
            },
            "#1860: deleted report/label stable visual-object duplicate-batch should preserve selected copied-row containing-section metadata");
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
                "\"deletedObjectCount\": 4"
            },
            "#1860: deleted report/label stable visual-object duplicate-batch should expose containing detail-band metadata");
    };

    const auto run_deleted_report_object_duplicate_batch_rollback = [&](const fs::path& asset_path,
                                                                        const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");
        const std::size_t before_count = visual_object_count(asset_path);

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-duplicate-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "middle-field-guid",
                "--new-unique-id", "mid-rb-copy-guid",
                "--selected-unique-id", "right-field-guid",
                "--new-unique-id", "mid-rb-copy-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1860: deleted report/label stable visual-object duplicate-batch identity collision should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectDuplicateBatch\": null",
                        "#1860: failed deleted report/label stable visual-object duplicate-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2225: failed deleted report/label stable visual-object duplicate-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2225: failed deleted report/label stable visual-object duplicate-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2181: failed deleted report/label stable visual-object duplicate-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed deleted report/label stable visual-object duplicate-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "The requested replacement identity already exists in the asset.",
                        "#1860: failed deleted report/label stable visual-object duplicate-batch JSON should report collision errors");
        expect(visual_object_count(asset_path) == before_count &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_exists(asset_path, "mid-rb-copy-guid") &&
                   visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1860: failed deleted report/label stable visual-object duplicate-batch should roll back earlier duplicates");
        (void)label;
    };

    run_deleted_report_object_duplicate_batch(temp_root / "deleted_report_object_duplicate_batch.frx",
                                              "deleted_report_object_duplicate_batch.frx",
                                              "report");
    run_deleted_report_object_duplicate_batch(temp_root / "deleted_report_object_duplicate_batch.lbx",
                                              "deleted_report_object_duplicate_batch.lbx",
                                              "label");
    run_deleted_report_object_duplicate_batch_rollback(temp_root / "deleted_report_object_duplicate_batch_rollback.frx",
                                                       "report");
    run_deleted_report_object_duplicate_batch_rollback(temp_root / "deleted_report_object_duplicate_batch_rollback.lbx",
                                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_visual_object_reorder_batch_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_report_object_reorder_batch = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const auto reorder_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-reorder-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "right-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--selected-unique-id", "middle-field-guid",
                "--placement", "after",
                "--target-unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object reorder-batch stdout:\n"
                      << reorder_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object reorder-batch stderr:\n"
                      << reorder_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_batch_process.exit_code == 0,
               "#1845: report/label stable visual-object reorder-batch JSON should exit successfully");
        expect_contains(reorder_batch_process.stdout_text, "\"visualObjectReorderBatch\": {",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose a batch object");
        expect_contains(reorder_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose affected object counts");
        expect_contains(reorder_batch_process.stdout_text, "\"dryRun\": false",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose committed state");
        expect_contains(reorder_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose mutation state");
        expect_contains(reorder_batch_process.stdout_text, "\"undoAvailable\": false",
                        "#1845: report/label stable visual-object reorder-batch JSON should expose undo availability");
        expect_contains(reorder_batch_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2171: report/label stable visual-object reorder-batch JSON should expose empty undo labels");
        expect(visual_object_order(asset_path) == "right-field-guid,middle-field-guid,left-field-guid",
               "#1845: report/label stable visual-object reorder-batch should apply ordered stable-selector moves");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#1845: report/label stable visual-object reorder-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1845: report/label stable visual-object reorder-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1845: label stable visual-object reorder-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2054: stable report/label live visual-object reorder-batch JSON should not fabricate deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2054: stable report/label live visual-object reorder-batch JSON should preserve zero deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"uniqueId\": \"right-field-guid\"",
                        "#1845: report/label stable visual-object reorder-batch should preserve selected object identity after reopen");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"right.value\""
            },
            "#1845: report/label stable visual-object reorder-batch should refresh selected reordered object metadata after reopen");
    };

    const auto run_report_object_reorder_batch_rollback = [&](const fs::path& asset_path,
                                                              const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-reorder-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "right-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--selected-unique-id", "middle-field-guid",
                "--placement", "after",
                "--target-unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1845: report/label stable visual-object reorder-batch missing target should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectReorderBatch\": null",
                        "#1845: failed report/label stable visual-object reorder-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2226: failed report/label stable visual-object reorder-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2226: failed report/label stable visual-object reorder-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2182: failed report/label stable visual-object reorder-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed report/label stable visual-object reorder-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1845: failed report/label stable visual-object reorder-batch JSON should report missing-target errors");
        expect(visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid",
               "#1845: failed report/label stable visual-object reorder-batch should roll back earlier reorder mutations");
        (void)label;
    };

    run_report_object_reorder_batch(temp_root / "report_object_reorder_batch.frx",
                                    "report_object_reorder_batch.frx",
                                    "report");
    run_report_object_reorder_batch(temp_root / "report_object_reorder_batch.lbx",
                                    "report_object_reorder_batch.lbx",
                                    "label");
    run_report_object_reorder_batch_rollback(temp_root / "report_object_reorder_batch_rollback.frx",
                                             "report");
    run_report_object_reorder_batch_rollback(temp_root / "report_object_reorder_batch_rollback.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_deleted_report_visual_object_batches_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_visual_object_reorder_batch_stable_json_tests";
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
               "#1861: deleted report/label reorder-batch fixture should start with deleted target rows");
    };

    const auto run_deleted_report_object_reorder_batch = [&](const fs::path& asset_path,
                                                             const std::string& title,
                                                             const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto reorder_batch_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-reorder-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "right-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--selected-unique-id", "middle-field-guid",
                "--placement", "after",
                "--target-unique-id", "right-field-guid",
                "--json"
            },
            temp_root);

        if (reorder_batch_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object reorder-batch stdout:\n"
                      << reorder_batch_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object reorder-batch stderr:\n"
                      << reorder_batch_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reorder_batch_process.exit_code == 0,
               "#1861: deleted report/label stable visual-object reorder-batch JSON should exit successfully");
        expect_contains(reorder_batch_process.stdout_text, "\"visualObjectReorderBatch\": {",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose a batch object");
        expect_contains(reorder_batch_process.stdout_text, "\"affectedObjectCount\": 2",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose affected object counts");
        expect_contains(reorder_batch_process.stdout_text, "\"dryRun\": false",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose committed state");
        expect_contains(reorder_batch_process.stdout_text, "\"mutatesAsset\": true",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose mutation state");
        expect_contains(reorder_batch_process.stdout_text, "\"undoAvailable\": false",
                        "#1861: deleted report/label stable visual-object reorder-batch JSON should expose undo availability");
        expect_contains(reorder_batch_process.stdout_text, "\"undoLabel\": \"\"",
                        "#2172: deleted report/label stable visual-object reorder-batch JSON should expose empty undo labels");
        expect(visual_object_order(asset_path) == "right-field-guid,middle-field-guid,left-field-guid" &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1861: deleted report/label stable visual-object reorder-batch should move deleted rows without changing deleted state");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "right-field-guid", "--json"},
            temp_root);

        if (reopen_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted report object reorder-batch reopen stdout:\n"
                      << reopen_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted report object reorder-batch reopen stderr:\n"
                      << reopen_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(reopen_process.exit_code == 0,
               "#1861: deleted report/label stable visual-object reorder-batch reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1861: deleted report/label stable visual-object reorder-batch should leave report-layout JSON readable");
        if (asset_path.extension() == ".lbx") {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#1861: deleted label stable visual-object reorder-batch should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview availability");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview width");
        expect_contains(reopen_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve live preview height");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should expose deleted preview availability");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview left bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview top bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview right bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview bottom bounds");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview width");
        expect_contains(reopen_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2055: stable deleted report/label visual-object reorder-batch JSON should preserve deleted preview height");
        expect_contains(reopen_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1861: deleted report/label stable visual-object reorder-batch should preserve live sibling counts");
        expect_contains(reopen_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1861: deleted report/label stable visual-object reorder-batch should preserve deleted object counts");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1861: deleted report/label stable visual-object reorder-batch should select the moved deleted row");
        expect_contains(reopen_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1861: deleted report/label stable visual-object reorder-batch should preserve containing sections");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1861: deleted report/label stable visual-object reorder-batch should preserve report object selection kind");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"right.value\"",
                "\"uniqueId\": \"right-field-guid\""
            },
            "#1861: deleted report/label stable visual-object reorder-batch should preserve selected moved-row containing-section metadata");
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
            "#1861: deleted report/label stable visual-object reorder-batch should expose containing detail-band metadata");
    };

    const auto run_deleted_report_object_reorder_batch_rollback = [&](const fs::path& asset_path,
                                                                      const std::string& label) {
        write_synthetic_report_table_for_layout_reorder_json(asset_path);
        mark_deleted(asset_path, "middle-field-guid");
        mark_deleted(asset_path, "right-field-guid");

        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--visual-object-reorder-batch",
                "--path", asset_path.string(),
                "--selected-unique-id", "right-field-guid",
                "--placement", "before",
                "--target-unique-id", "left-field-guid",
                "--selected-unique-id", "middle-field-guid",
                "--placement", "after",
                "--target-unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1861: deleted report/label stable visual-object reorder-batch missing target should fail");
        expect_contains(rollback_process.stdout_text, "\"visualObjectReorderBatch\": null",
                        "#1861: failed deleted report/label stable visual-object reorder-batch JSON should not expose stale batch objects");
        expect_not_contains(rollback_process.stdout_text, "\"dryRun\": false",
                            "#2227: failed deleted report/label stable visual-object reorder-batch JSON should not expose stale committed state");
        expect_not_contains(rollback_process.stdout_text, "\"mutatesAsset\": true",
                            "#2227: failed deleted report/label stable visual-object reorder-batch JSON should not expose stale mutation state");
        expect_not_contains(rollback_process.stdout_text, "\"undoAvailable\": true",
                            "#2183: failed deleted report/label stable visual-object reorder-batch JSON should not advertise undo availability");
        expect_not_contains(rollback_process.stdout_text, "\"undoLabel\":",
                            "#2199: failed deleted report/label stable visual-object reorder-batch JSON should not expose stale undo labels");
        expect_contains(rollback_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1861: failed deleted report/label stable visual-object reorder-batch JSON should report missing-target errors");
        expect(visual_object_order(asset_path) == "left-field-guid,middle-field-guid,right-field-guid" &&
                   visual_object_deleted(asset_path, "middle-field-guid") &&
                   visual_object_deleted(asset_path, "right-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid"),
               "#1861: failed deleted report/label stable visual-object reorder-batch should roll back earlier reorder mutations");
        (void)label;
    };

    run_deleted_report_object_reorder_batch(temp_root / "deleted_report_object_reorder_batch.frx",
                                            "deleted_report_object_reorder_batch.frx",
                                            "report");
    run_deleted_report_object_reorder_batch(temp_root / "deleted_report_object_reorder_batch.lbx",
                                            "deleted_report_object_reorder_batch.lbx",
                                            "label");
    run_deleted_report_object_reorder_batch_rollback(temp_root / "deleted_report_object_reorder_batch_rollback.frx",
                                                     "report");
    run_deleted_report_object_reorder_batch_rollback(temp_root / "deleted_report_object_reorder_batch_rollback.lbx",
                                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
