void test_studio_host_json_applies_report_object_subtree_deleted_state_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_object_subtree_deleted_state_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_subtree_delete_restore = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--subtree-deleted-state",
                "--subtree-deleted", "true",
                "--unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object subtree delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object subtree delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1857: report/label stable object subtree delete should exit successfully");
        expect(visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid"),
               "#1857: report/label stable object subtree delete should mark only the selected flat layout row");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1857: report/label stable object subtree delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1857: label stable object subtree delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview width");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2045: stable report/label object subtree delete JSON should preserve live preview height");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 100",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2800",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 50",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview width");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                        "#2045: stable report/label object subtree delete JSON should expose deleted preview height");
        expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 2",
                        "#1857: report/label stable object subtree delete should remove the object from live counts");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1857: report/label stable object subtree delete should expose deleted object counts");
        expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1857: report/label stable object subtree delete should leave the report section selection empty");
        expect_contains(delete_process.stdout_text, "\"selectedReportSection\": null",
                        "#1857: report/label stable object subtree delete should serialize a null report section selection");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1857: report/label stable object subtree delete should preserve selected-object availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1857: report/label stable object subtree delete should preserve containing-section availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1857: report/label stable object subtree delete should preserve report object selection kind");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1857: report/label stable object subtree delete should serialize selected deleted-object metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_1\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 2",
                "\"deletedObjectCount\": 1"
            },
            "#1857: report/label stable object subtree delete should expose containing detail-band metadata");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--subtree-deleted-state",
                "--subtree-deleted", "false",
                "--unique-id", "middle-field-guid",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable report object subtree restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable report object subtree restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1857: report/label stable object subtree restore should exit successfully");
        expect(!visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid"),
               "#1857: report/label stable object subtree restore should restore the selected row and preserve siblings");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1857: report/label stable object subtree restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1857: label stable object subtree restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 7000",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview width");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#2045: stable report/label object subtree restore JSON should preserve live preview height");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2045: stable report/label object subtree restore JSON should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview left bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview top bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview right bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview width");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#2045: stable report/label object subtree restore JSON should preserve zero deleted preview height");
        expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1857: report/label stable object subtree restore should restore live object counts");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1857: report/label stable object subtree restore should clear deleted object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1857: report/label stable object subtree restore should refresh containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1857: report/label stable object subtree restore should preserve report object selection kind");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 3",
                "\"objectKind\": \"field\"",
                "\"expression\": \"middle.value\"",
                "\"uniqueId\": \"middle-field-guid\""
            },
            "#1857: report/label stable object subtree restore should refresh selected live-object metadata");
    };

    const auto run_missing_selector = [&](const fs::path& asset_path,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_subtree_deleted_state_json(asset_path);
        const auto missing_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--subtree-deleted-state",
                "--subtree-deleted", "true",
                "--unique-id", "missing-guid",
                "--json"
            },
            temp_root);

        expect(missing_process.exit_code == 4,
               "#1857: report/label stable object subtree delete should reject missing stable selectors");
        expect_contains(missing_process.stdout_text, "status: error",
                        "#1857: missing-selector report/label subtree delete should report error status");
        expect_contains(missing_process.stdout_text, "No visual object with the requested unique id was found.",
                        "#1857: missing-selector report/label subtree delete should report selector errors");
        expect(!visual_object_deleted(asset_path, "left-field-guid") &&
                   !visual_object_deleted(asset_path, "middle-field-guid") &&
                   !visual_object_deleted(asset_path, "right-field-guid"),
               "#1857: failed report/label stable object subtree delete should not mutate layout rows");
        (void)label;
    };

    run_subtree_delete_restore(temp_root / "object_subtree_deleted_state.frx",
                               "object_subtree_deleted_state.frx",
                               "report");
    run_subtree_delete_restore(temp_root / "object_subtree_deleted_state.lbx",
                               "object_subtree_deleted_state.lbx",
                               "label");
    run_missing_selector(temp_root / "object_subtree_deleted_state_missing.frx",
                         "report");
    run_missing_selector(temp_root / "object_subtree_deleted_state_missing.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
