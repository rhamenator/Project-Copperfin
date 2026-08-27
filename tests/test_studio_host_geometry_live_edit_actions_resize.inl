void test_studio_host_json_resizes_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_resize_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_resize = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "field-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " resize live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " resize live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1626: live report/label layout object resize geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1626: live report/label layout object resize geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto resize_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "field-guid",
                "--resize-object",
                "--resize-mode", "size",
                "--anchor-unique-id", "label-guid",
                "--resize-target-unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (resize_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout resize stdout:\n"
                      << resize_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout resize stderr:\n"
                      << resize_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(resize_process.exit_code == 0,
               "#1626: live edited report/label layout object resize should exit successfully");
        expect(visual_object_property(asset_path, "field-guid", "HPOS") == "300" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "1800" &&
                   visual_object_property(asset_path, "field-guid", "HEIGHT") == "350",
               "#1626: live edited report/label layout object resize should apply anchor size and preserve edited left position");
        expect(visual_object_property(asset_path, "label-guid", "WIDTH") == "1800" &&
                   visual_object_property(asset_path, "label-guid", "HEIGHT") == "350",
               "#1626: live edited report/label layout object resize should preserve anchor size");
        expect_contains(resize_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1626: live edited report/label layout object resize should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(resize_process.stdout_text, "\"isLabel\": true",
                            "#1626: live edited label layout object resize should retain label identity");
        }
        expect_contains(resize_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1885: live edited report/label layout object resize should preserve preview availability");
        expect_contains(resize_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1885: live edited report/label layout object resize should preserve preview left bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1885: live edited report/label layout object resize should preserve preview top bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1885: live edited report/label layout object resize should preserve preview right bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1885: live edited report/label layout object resize should preserve preview bottom bounds");
        expect_contains(resize_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1885: live edited report/label layout object resize should preserve preview widths");
        expect_contains(resize_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1885: live edited report/label layout object resize should preserve preview heights");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1885: live edited report/label layout object resize should preserve deleted preview availability");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1885: live edited report/label layout object resize should preserve deleted preview left bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1885: live edited report/label layout object resize should preserve deleted preview top bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1885: live edited report/label layout object resize should preserve deleted preview right bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1885: live edited report/label layout object resize should preserve deleted preview bottom bounds");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1885: live edited report/label layout object resize should preserve deleted preview widths");
        expect_contains(resize_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1885: live edited report/label layout object resize should preserve deleted preview heights");
        expect_contains(resize_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1626: live edited report/label layout object resize should preserve selected-object availability");
        expect_contains(resize_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1626: live edited report/label layout object resize should preserve containing-section availability");
        expect_contains(resize_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1626: live edited report/label layout object resize should serialize containing-section metadata");
        expect_contains_in_order(
            resize_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 950",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"left\": 300",
                "\"top\": 2600",
                "\"width\": 1800",
                "\"right\": 2100",
                "\"height\": 350",
                "\"bottom\": 2950"
            },
            "#1626: live edited report/label layout object resize should refresh selected resized geometry and section metadata");
    };

    run_live_edited_resize(temp_root / "resize_live_edited.frx",
                           "resize_live_edited.frx",
                           "report");
    run_live_edited_resize(temp_root / "resize_live_edited.lbx",
                           "resize_live_edited.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
