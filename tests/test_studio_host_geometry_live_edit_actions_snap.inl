void test_studio_host_json_snaps_live_edited_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_snap_live_edited_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_live_edited_snap = [&](const fs::path& asset_path,
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
                std::cerr << "studio host " << label << " snap live " << property_name
                          << " update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " snap live " << property_name
                          << " update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1627: live report/label layout object snap geometry update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1627: live report/label layout object snap geometry update should preserve live state");
        };

        set_live_geometry("HPOS", "375");
        set_live_geometry("VPOS", "2550");
        set_live_geometry("WIDTH", "250");
        set_live_geometry("HEIGHT", "500");

        const auto snap_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "field-guid",
                "--snap-object",
                "--snap-mode", "both",
                "--grid-width", "700",
                "--grid-height", "750",
                "--snap-target-unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (snap_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout snap stdout:\n"
                      << snap_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout snap stderr:\n"
                      << snap_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(snap_process.exit_code == 0,
               "#1627: live edited report/label layout object snap should exit successfully");
        expect(visual_object_property(asset_path, "field-guid", "HPOS") == "700" &&
                   visual_object_property(asset_path, "field-guid", "VPOS") == "2250" &&
                   visual_object_property(asset_path, "field-guid", "WIDTH") == "250" &&
                   visual_object_property(asset_path, "field-guid", "HEIGHT") == "500",
               "#1627: live edited report/label layout object snap should apply grid position and preserve edited size fields");
        expect_contains(snap_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1627: live edited report/label layout object snap should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(snap_process.stdout_text, "\"isLabel\": true",
                            "#1627: live edited label layout object snap should retain label identity");
        }
        expect_contains(snap_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1886: live edited report/label layout object snap should preserve preview availability");
        expect_contains(snap_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1886: live edited report/label layout object snap should preserve preview left bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1886: live edited report/label layout object snap should preserve preview top bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1886: live edited report/label layout object snap should preserve preview right bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1886: live edited report/label layout object snap should preserve preview bottom bounds");
        expect_contains(snap_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1886: live edited report/label layout object snap should preserve preview widths");
        expect_contains(snap_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1886: live edited report/label layout object snap should preserve preview heights");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1886: live edited report/label layout object snap should preserve deleted preview availability");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1886: live edited report/label layout object snap should preserve deleted preview left bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1886: live edited report/label layout object snap should preserve deleted preview top bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1886: live edited report/label layout object snap should preserve deleted preview right bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1886: live edited report/label layout object snap should preserve deleted preview bottom bounds");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1886: live edited report/label layout object snap should preserve deleted preview widths");
        expect_contains(snap_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1886: live edited report/label layout object snap should preserve deleted preview heights");
        expect_contains(snap_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1627: live edited report/label layout object snap should preserve selected-object availability");
        expect_contains(snap_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1627: live edited report/label layout object snap should preserve containing-section availability");
        expect_contains(snap_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1627: live edited report/label layout object snap should serialize containing-section metadata");
        expect_contains_in_order(
            snap_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 250",
                "\"sectionRelativeBottom\": 750",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"left\": 700",
                "\"top\": 2250",
                "\"width\": 250",
                "\"right\": 950",
                "\"height\": 500",
                "\"bottom\": 2750"
            },
            "#1627: live edited report/label layout object snap should refresh selected snapped geometry and section metadata");
    };

    run_live_edited_snap(temp_root / "snap_live_edited.frx",
                         "snap_live_edited.frx",
                         "report");
    run_live_edited_snap(temp_root / "snap_live_edited.lbx",
                         "snap_live_edited.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
