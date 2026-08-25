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
