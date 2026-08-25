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

