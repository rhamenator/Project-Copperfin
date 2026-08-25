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

