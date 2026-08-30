#if !defined(COPPERFIN_REPORT_DELETED_STATES_SKIP_HOST_SMOKE)
void test_studio_host_json_applies_report_deleted_states_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_deleted_states_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_batch_delete = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);
        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted-states batch delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted-states batch delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1839: report/label stable deleted-states batch delete should exit successfully");
        expect(dbf_record_deleted(asset_path, 0U) && dbf_record_deleted(asset_path, 1U),
               "#1839: report/label stable deleted-states batch delete should mark settings and section rows deleted");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1839: report/label stable deleted-states batch delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1839: label stable deleted-states batch delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should expose live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 100",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve retained live-object top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve live preview width");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8000",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve retained live-object preview height");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should expose deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should preserve deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2700",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh deleted preview width");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2900",
                        "#2042: stable report/label settings+section deleted-states batch delete JSON should refresh deleted preview height");
        expect_contains(delete_process.stdout_text, "\"settingCount\": 0",
                        "#1839: report/label stable deleted-states batch delete should remove live settings");
        expect_contains(delete_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1839: report/label stable deleted-states batch delete should expose deleted settings");
        expect_contains(delete_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1839: report/label stable deleted-states batch delete should expose effective deleted-root page setup");
        expect_contains(delete_process.stdout_text, "\"orientationAvailable\": true",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root orientation availability");
        expect_contains(delete_process.stdout_text, "\"orientationCode\": 0",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root orientation");
        expect_contains(delete_process.stdout_text, "\"paperSizeAvailable\": true",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root paper-size availability");
        expect_contains(delete_process.stdout_text, "\"paperSizeCode\": 1",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root paper size");
        expect_contains(delete_process.stdout_text, "\"topMarginAvailable\": true",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root top-margin availability");
        expect_contains(delete_process.stdout_text, "\"topMargin\": 10",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root top margin");
        expect_contains(delete_process.stdout_text, "\"bottomMarginAvailable\": true",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root bottom-margin availability");
        expect_contains(delete_process.stdout_text, "\"bottomMargin\": 20",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root bottom margin");
        expect_contains(delete_process.stdout_text, "\"gridVerticalAvailable\": true",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root vertical-grid availability");
        expect_contains(delete_process.stdout_text, "\"gridVertical\": 4",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root vertical grid");
        expect_contains(delete_process.stdout_text, "\"gridHorizontalAvailable\": true",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root horizontal-grid availability");
        expect_contains(delete_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1839: report/label stable deleted-states batch delete should expose deleted-root horizontal grid");
        expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                        "#1839: report/label stable deleted-states batch delete should remove the section from live counts");
        expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1839: report/label stable deleted-states batch delete should expose deleted section counts");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0"
            },
            "#1839: report/label stable deleted-states batch delete should move settings into deleted metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": true"
            },
            "#1839: report/label stable deleted-states batch delete should move the section into deleted metadata");
        expect_contains(delete_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1839: report/label stable deleted-states batch delete should not fabricate selected settings");
        expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1839: report/label stable deleted-states batch delete should not fabricate selected sections");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1839: report/label stable deleted-states batch delete should not fabricate a report selection");
    };

    const auto run_batch_restore = [&](const fs::path& asset_path,
                                       const std::string& title,
                                       const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_settings_and_section_json(asset_path);
        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "false",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "false",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted-states batch restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted-states batch restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1839: report/label stable deleted-states batch restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 0U) && !dbf_record_deleted(asset_path, 1U),
               "#1839: report/label stable deleted-states batch restore should restore settings and section rows");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1839: report/label stable deleted-states batch restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1839: label stable deleted-states batch restore should retain label identity");
        }
        expect_full_report_layout_preview_bounds(
            restore_process.stdout_text,
            "#2042: stable report/label settings+section deleted-states batch restore JSON");
        expect_contains(restore_process.stdout_text, "\"settingCount\": 6",
                        "#1839: report/label stable deleted-states batch restore should restore live settings");
        expect_contains(restore_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1839: report/label stable deleted-states batch restore should clear deleted settings");
        expect_contains(restore_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1839: report/label stable deleted-states batch restore should restore live page setup");
        expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                        "#1839: report/label stable deleted-states batch restore should restore live section counts");
        expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1839: report/label stable deleted-states batch restore should clear deleted section counts");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0"
            },
            "#1839: report/label stable deleted-states batch restore should move settings into live metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1839: report/label stable deleted-states batch restore should move the section into live metadata");
        expect_contains(restore_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1839: report/label stable deleted-states batch restore should not fabricate selected settings");
        expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1839: report/label stable deleted-states batch restore should not fabricate selected sections");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1839: report/label stable deleted-states batch restore should not fabricate a report selection");
    };

    const auto run_batch_rollback = [&](const fs::path& asset_path,
                                        const std::string& label) {
        write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "missing-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1839: report/label stable deleted-states missing-target batch should fail");
        expect(!dbf_record_deleted(asset_path, 0U) && !dbf_record_deleted(asset_path, 1U),
               "#1839: failed report/label stable deleted-states batch should roll back earlier mutations");
        expect_contains(rollback_process.stdout_text, "status: error",
                        "#1839: failed report/label stable deleted-states batch should report JSON error status");
        expect_contains(rollback_process.stdout_text, "error",
                        "#1839: failed report/label stable deleted-states batch should report an error message");
        if (asset_path.extension() == ".lbx") {
            expect(!dbf_record_deleted(asset_path, 0U) && !dbf_record_deleted(asset_path, 1U),
                   "#1839: failed label stable deleted-states batch should preserve label settings and section rows");
        }
        (void)label;
    };

    run_batch_delete(temp_root / "deleted_states_delete_stable.frx",
                     "deleted_states_delete_stable.frx",
                     "report");
    run_batch_delete(temp_root / "deleted_states_delete_stable.lbx",
                     "deleted_states_delete_stable.lbx",
                     "label");
    run_batch_restore(temp_root / "deleted_states_restore_stable.frx",
                      "deleted_states_restore_stable.frx",
                      "report");
    run_batch_restore(temp_root / "deleted_states_restore_stable.lbx",
                      "deleted_states_restore_stable.lbx",
                      "label");
    run_batch_rollback(temp_root / "deleted_states_rollback_stable.frx",
                       "report");
    run_batch_rollback(temp_root / "deleted_states_rollback_stable.lbx",
                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_applies_report_object_deleted_states_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_object_deleted_states_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_object_batch_delete = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "label-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable object deleted-states batch delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable object deleted-states batch delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1840: report/label stable object deleted-states batch delete should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid") &&
                   visual_object_deleted(asset_path, "label-guid"),
               "#1840: report/label stable object deleted-states batch delete should mark both object rows deleted");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1840: report/label stable object deleted-states batch delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1840: label stable object deleted-states batch delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh live preview width");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#2043: stable report/label object deleted-states batch delete JSON should preserve live preview height");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2043: stable report/label object deleted-states batch delete JSON should expose deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 900",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 5200",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3050",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 4300",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview width");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2950",
                        "#2043: stable report/label object deleted-states batch delete JSON should refresh deleted preview height");
        expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1840: report/label stable object deleted-states batch delete should remove objects from live counts");
        expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 0",
                        "#1840: report/label stable object deleted-states batch delete should remove placed live objects");
        expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1840: report/label stable object deleted-states batch delete should preserve unrelated unplaced objects");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 3",
                        "#1840: report/label stable object deleted-states batch delete should expose deleted object counts");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"objectKind\": \"field\"",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"objectKind\": \"label\""
            },
            "#1840: report/label stable object deleted-states batch delete should move both objects into deleted metadata");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1840: report/label stable object deleted-states batch delete should not fabricate selected objects");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1840: report/label stable object deleted-states batch delete should not fabricate containing sections");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1840: report/label stable object deleted-states batch delete should not fabricate a report selection");
    };

    const auto run_object_batch_restore = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto field_delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "field-guid",
            .deleted = true
        });
        const auto label_delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "label-guid",
            .deleted = true
        });
        expect(field_delete_result.ok && label_delete_result.ok &&
                   visual_object_deleted(asset_path, "field-guid") &&
                   visual_object_deleted(asset_path, "label-guid"),
               "#1840: report/label stable object deleted-states restore fixture should start deleted");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "false",
                "--deleted-state-target-unique-id", "label-guid",
                "--deleted-state", "false",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable object deleted-states batch restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable object deleted-states batch restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1840: report/label stable object deleted-states batch restore should exit successfully");
        expect(!visual_object_deleted(asset_path, "field-guid") &&
                   !visual_object_deleted(asset_path, "label-guid"),
               "#1840: report/label stable object deleted-states batch restore should restore both object rows");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1840: report/label stable object deleted-states batch restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1840: label stable object deleted-states batch restore should retain label identity");
        }
        expect_full_report_layout_preview_bounds(
            restore_process.stdout_text,
            "#2043: stable report/label object deleted-states batch restore JSON");
        expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1840: report/label stable object deleted-states batch restore should restore live object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1840: report/label stable object deleted-states batch restore should restore placed object counts");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1840: report/label stable object deleted-states batch restore should preserve unrelated unplaced objects");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1840: report/label stable object deleted-states batch restore should clear restored deleted objects");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"sections\": [",
                "\"recordIndex\": 1",
                "\"objectCount\": 1",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"objectKind\": \"label\"",
                "\"recordIndex\": 2",
                "\"objectCount\": 1",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"objectKind\": \"field\""
            },
            "#1840: report/label stable object deleted-states batch restore should move both objects into live section metadata");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1840: report/label stable object deleted-states batch restore should not fabricate selected objects");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1840: report/label stable object deleted-states batch restore should not fabricate containing sections");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1840: report/label stable object deleted-states batch restore should not fabricate a report selection");
    };

    const auto run_object_batch_rollback = [&](const fs::path& asset_path,
                                               const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "missing-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1840: report/label stable object deleted-states missing-target batch should fail");
        expect(!visual_object_deleted(asset_path, "field-guid") &&
                   !visual_object_deleted(asset_path, "label-guid"),
               "#1840: failed report/label stable object deleted-states batch should roll back earlier mutations");
        expect_contains(rollback_process.stdout_text, "status: error",
                        "#1840: failed report/label stable object deleted-states batch should report JSON error status");
        expect_contains(rollback_process.stdout_text, "error",
                        "#1840: failed report/label stable object deleted-states batch should report an error message");
        (void)label;
    };

    run_object_batch_delete(temp_root / "object_deleted_states_delete.frx",
                            "object_deleted_states_delete.frx",
                            "report");
    run_object_batch_delete(temp_root / "object_deleted_states_delete.lbx",
                            "object_deleted_states_delete.lbx",
                            "label");
    run_object_batch_restore(temp_root / "object_deleted_states_restore.frx",
                             "object_deleted_states_restore.frx",
                             "report");
    run_object_batch_restore(temp_root / "object_deleted_states_restore.lbx",
                             "object_deleted_states_restore.lbx",
                             "label");
    run_object_batch_rollback(temp_root / "object_deleted_states_rollback.frx",
                              "report");
    run_object_batch_rollback(temp_root / "object_deleted_states_rollback.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#include "test_studio_host_json_report_object_deleted_subtree.inl"

void test_studio_host_json_applies_mixed_report_deleted_states_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_mixed_report_deleted_states_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_mixed_batch_delete = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);
        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable mixed deleted-states batch delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable mixed deleted-states batch delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1841: report/label stable mixed deleted-states batch delete should exit successfully");
        expect(dbf_record_deleted(asset_path, 0U) &&
                   dbf_record_deleted(asset_path, 1U) &&
                   visual_object_deleted(asset_path, "field-guid"),
               "#1841: report/label stable mixed deleted-states batch delete should mark settings, section, and object rows deleted");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1841: report/label stable mixed deleted-states batch delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1841: label stable mixed deleted-states batch delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 100",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve retained live-object top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve retained live-object right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve retained live-object preview width");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8000",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should preserve retained live-object preview height");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should expose deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 5200",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3050",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 5200",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview width");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 3050",
                        "#2044: stable report/label mixed deleted-states batch delete JSON should refresh deleted preview height");
        expect_contains(delete_process.stdout_text, "\"settingCount\": 0",
                        "#1841: report/label stable mixed deleted-states batch delete should remove live settings");
        expect_contains(delete_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted settings");
        expect_contains(delete_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1841: report/label stable mixed deleted-states batch delete should expose effective deleted-root page setup");
        expect_contains(delete_process.stdout_text, "\"orientationAvailable\": true",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root orientation availability");
        expect_contains(delete_process.stdout_text, "\"orientationCode\": 0",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root orientation");
        expect_contains(delete_process.stdout_text, "\"paperSizeAvailable\": true",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root paper-size availability");
        expect_contains(delete_process.stdout_text, "\"paperSizeCode\": 1",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root paper size");
        expect_contains(delete_process.stdout_text, "\"topMarginAvailable\": true",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root top-margin availability");
        expect_contains(delete_process.stdout_text, "\"topMargin\": 10",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root top margin");
        expect_contains(delete_process.stdout_text, "\"bottomMarginAvailable\": true",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root bottom-margin availability");
        expect_contains(delete_process.stdout_text, "\"bottomMargin\": 20",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root bottom margin");
        expect_contains(delete_process.stdout_text, "\"gridVerticalAvailable\": true",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root vertical-grid availability");
        expect_contains(delete_process.stdout_text, "\"gridVertical\": 4",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root vertical grid");
        expect_contains(delete_process.stdout_text, "\"gridHorizontalAvailable\": true",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root horizontal-grid availability");
        expect_contains(delete_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted-root horizontal grid");
        expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch delete should remove the selected section from live counts");
        expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted section counts");
        expect_contains(delete_process.stdout_text, "\"liveObjectCount\": 2",
                        "#1841: report/label stable mixed deleted-states batch delete should remove the selected object from live counts");
        expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch delete should preserve placed live objects");
        expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch delete should preserve unplaced live objects");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1841: report/label stable mixed deleted-states batch delete should expose deleted object counts");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0"
            },
            "#1841: report/label stable mixed deleted-states batch delete should move settings into deleted metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": true"
            },
            "#1841: report/label stable mixed deleted-states batch delete should move the section into deleted metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"objectKind\": \"field\""
            },
            "#1841: report/label stable mixed deleted-states batch delete should move the object into deleted metadata");
        expect_contains(delete_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch delete should not fabricate selected settings");
        expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch delete should not fabricate selected sections");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch delete should not fabricate selected objects");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1841: report/label stable mixed deleted-states batch delete should not fabricate a report selection");
    };

    const auto run_mixed_batch_restore = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_settings_and_section_json(asset_path);
        const auto field_delete_result = copperfin::vfp::set_visual_object_deleted_state({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "field-guid",
            .deleted = true
        });
        expect(field_delete_result.ok &&
                   dbf_record_deleted(asset_path, 0U) &&
                   dbf_record_deleted(asset_path, 1U) &&
                   visual_object_deleted(asset_path, "field-guid"),
               "#1841: report/label stable mixed deleted-states restore fixture should start deleted");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "false",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "false",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "false",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable mixed deleted-states batch restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable mixed deleted-states batch restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1841: report/label stable mixed deleted-states batch restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 0U) &&
                   !dbf_record_deleted(asset_path, 1U) &&
                   !visual_object_deleted(asset_path, "field-guid"),
               "#1841: report/label stable mixed deleted-states batch restore should restore settings, section, and object rows");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1841: report/label stable mixed deleted-states batch restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1841: label stable mixed deleted-states batch restore should retain label identity");
        }
        expect_full_report_layout_preview_bounds(
            restore_process.stdout_text,
            "#2044: stable report/label mixed deleted-states batch restore JSON");
        expect_contains(restore_process.stdout_text, "\"settingCount\": 6",
                        "#1841: report/label stable mixed deleted-states batch restore should restore live settings");
        expect_contains(restore_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1841: report/label stable mixed deleted-states batch restore should clear deleted settings");
        expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                        "#1841: report/label stable mixed deleted-states batch restore should restore live sections");
        expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1841: report/label stable mixed deleted-states batch restore should clear deleted sections");
        expect_contains(restore_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1841: report/label stable mixed deleted-states batch restore should restore live object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1841: report/label stable mixed deleted-states batch restore should restore placed object counts");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch restore should preserve unrelated unplaced objects");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1841: report/label stable mixed deleted-states batch restore should clear restored deleted objects");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0"
            },
            "#1841: report/label stable mixed deleted-states batch restore should move settings into live metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1841: report/label stable mixed deleted-states batch restore should move the section into live metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"sections\": [",
                "\"recordIndex\": 2",
                "\"objectCount\": 1",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"objectKind\": \"field\""
            },
            "#1841: report/label stable mixed deleted-states batch restore should move the object into live section metadata");
        expect_contains(restore_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch restore should not fabricate selected settings");
        expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch restore should not fabricate selected sections");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1841: report/label stable mixed deleted-states batch restore should not fabricate selected objects");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1841: report/label stable mixed deleted-states batch restore should not fabricate a report selection");
    };

    const auto run_mixed_batch_rollback = [&](const fs::path& asset_path,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_settings_and_section_json(asset_path);
        const auto rollback_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--deleted-states",
                "--deleted-state-target-unique-id", "settings-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "section-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "field-guid",
                "--deleted-state", "true",
                "--deleted-state-target-unique-id", "missing-guid",
                "--deleted-state", "true",
                "--json"
            },
            temp_root);

        expect(rollback_process.exit_code == 4,
               "#1841: report/label stable mixed deleted-states missing-target batch should fail");
        expect(!dbf_record_deleted(asset_path, 0U) &&
                   !dbf_record_deleted(asset_path, 1U) &&
                   !visual_object_deleted(asset_path, "field-guid"),
               "#1841: failed report/label stable mixed deleted-states batch should roll back earlier mutations");
        expect_contains(rollback_process.stdout_text, "status: error",
                        "#1841: failed report/label stable mixed deleted-states batch should report JSON error status");
        expect_contains(rollback_process.stdout_text, "error",
                        "#1841: failed report/label stable mixed deleted-states batch should report an error message");
        (void)label;
    };

    run_mixed_batch_delete(temp_root / "mixed_deleted_states_delete.frx",
                           "mixed_deleted_states_delete.frx",
                           "report");
    run_mixed_batch_delete(temp_root / "mixed_deleted_states_delete.lbx",
                           "mixed_deleted_states_delete.lbx",
                           "label");
    run_mixed_batch_restore(temp_root / "mixed_deleted_states_restore.frx",
                            "mixed_deleted_states_restore.frx",
                            "report");
    run_mixed_batch_restore(temp_root / "mixed_deleted_states_restore.lbx",
                            "mixed_deleted_states_restore.lbx",
                            "label");
    run_mixed_batch_rollback(temp_root / "mixed_deleted_states_rollback.frx",
                             "report");
    run_mixed_batch_rollback(temp_root / "mixed_deleted_states_rollback.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif
