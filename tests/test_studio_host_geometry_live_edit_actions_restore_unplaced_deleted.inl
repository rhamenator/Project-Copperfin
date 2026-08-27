void test_studio_host_json_restores_live_edited_unplaced_then_deleted_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_restore_live_edited_unplaced_deleted_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_unplaced_delete_restore = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1618: live edited unplaced then deleted report/label layout object restore fixture should start live");

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "3",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " live unplaced round-trip "
                          << property_name << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live unplaced round-trip "
                          << property_name << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1618: live unplaced report/label layout object round-trip geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1618: live unplaced report/label layout object round-trip geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--delete-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited unplaced delete before restore stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited unplaced delete before restore stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1618: live edited unplaced report/label layout object delete before restore should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1618: live edited unplaced report/label layout object delete before restore should mark the DBF record deleted");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--restore-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " live edited unplaced delete restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live edited unplaced delete restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1618: live edited unplaced then deleted report/label layout object restore should exit successfully");
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1618: live edited unplaced then deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "-300" &&
                   top_property.ok && top_property.exists && top_property.value == "9000" &&
                   height_property.ok && height_property.exists && height_property.value == "700",
               "#1618: live edited unplaced then deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1618: live edited unplaced then deleted report/label layout object restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1890: edited unplaced label layout object restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1890: edited unplaced report/label layout object restore should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": -300",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1890: edited unplaced report/label layout object restore should preserve live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 3700",
                        "#1890: edited unplaced report/label layout object restore should expand preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 9700",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 4000",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 9700",
                        "#1618: live edited unplaced then deleted report/label layout object restore should expand preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview left bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview top bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview right bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview widths");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1890: edited unplaced report/label layout object restore should preserve deleted preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1618: live edited unplaced then deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1618: live edited unplaced then deleted report/label layout object restore should not count out-of-band restored objects as placed");
        expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1618: live edited unplaced then deleted report/label layout object restore should add restored objects to unplaced counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1618: live edited unplaced then deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1618: live edited unplaced then deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1618: live edited unplaced then deleted report/label layout object restore should not fabricate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1618: live edited unplaced then deleted report/label layout object restore should serialize null containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 4000",
                "\"right\": 3700",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1618: live edited unplaced then deleted report/label layout object restore should refresh selected unplaced geometry without section metadata");
    };

    run_live_edited_unplaced_delete_restore(temp_root / "restore_live_edited_unplaced_deleted.frx",
                                            "restore_live_edited_unplaced_deleted.frx",
                                            "report");
    run_live_edited_unplaced_delete_restore(temp_root / "restore_live_edited_unplaced_deleted.lbx",
                                            "restore_live_edited_unplaced_deleted.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
