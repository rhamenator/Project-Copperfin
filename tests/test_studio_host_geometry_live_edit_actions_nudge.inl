void test_studio_host_json_nudges_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_nudge_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_nudge = [&](const fs::path& asset_path,
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
                std::cerr << "studio host " << label << " nudge live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " nudge live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1628: live report/label layout object nudge geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1628: live report/label layout object nudge geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "300");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto nudge_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "field-guid",
                "--nudge-object",
                "--nudge-mode", "both",
                "--delta-hpos", "25",
                "--delta-vpos", "-100",
                "--nudge-target-unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (nudge_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout nudge stdout:\n"
                      << nudge_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout nudge stderr:\n"
                      << nudge_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(nudge_process.exit_code == 0,
               "#1628: live edited report/label layout object nudge should exit successfully");
        expect(visual_object_property(asset_path, "field-guid", "HPOS") == "325" &&
                   visual_object_property(asset_path, "field-guid", "VPOS") == "2500" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "field-guid", "HEIGHT") == "500",
               "#1628: live edited report/label layout object nudge should apply delta position and preserve edited size fields");
        expect_contains(nudge_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1628: live edited report/label layout object nudge should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(nudge_process.stdout_text, "\"isLabel\": true",
                            "#1628: live edited label layout object nudge should retain label identity");
        }
        expect_contains(nudge_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1883: live edited report/label layout object nudge should preserve preview availability");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1883: live edited report/label layout object nudge should preserve preview left bounds");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1883: live edited report/label layout object nudge should preserve preview top bounds");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1883: live edited report/label layout object nudge should preserve preview right bounds");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1883: live edited report/label layout object nudge should preserve preview bottom bounds");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1883: live edited report/label layout object nudge should preserve preview widths");
        expect_contains(nudge_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1883: live edited report/label layout object nudge should preserve preview heights");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview availability");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview left bounds");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview top bounds");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview right bounds");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview bottom bounds");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview widths");
        expect_contains(nudge_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1883: live edited report/label layout object nudge should preserve deleted preview heights");
        expect_contains(nudge_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1628: live edited report/label layout object nudge should preserve selected-object availability");
        expect_contains(nudge_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1628: live edited report/label layout object nudge should preserve containing-section availability");
        expect_contains(nudge_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1628: live edited report/label layout object nudge should serialize containing-section metadata");
        expect_contains_in_order(
            nudge_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 500",
                "\"sectionRelativeBottom\": 1000",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"left\": 325",
                "\"top\": 2500",
                "\"width\": 250",
                "\"right\": 575",
                "\"height\": 500",
                "\"bottom\": 3000"
            },
            "#1628: live edited report/label layout object nudge should refresh selected nudged geometry and section metadata");
    };

    run_live_edited_nudge(temp_root / "nudge_live_edited.frx",
                          "nudge_live_edited.frx",
                          "report");
    run_live_edited_nudge(temp_root / "nudge_live_edited.lbx",
                          "nudge_live_edited.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
