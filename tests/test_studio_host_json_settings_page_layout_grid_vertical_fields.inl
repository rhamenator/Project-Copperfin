void test_studio_host_json_updates_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_grid_vertical_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_grid_vertical_update = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& updated_grid,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_grid_vertical_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "GRIDV",
                "--property-value", updated_grid,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable vertical-grid field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable vertical-grid field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1831: report/label stable vertical-grid field update should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "GRIDV"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value == updated_grid,
               "#1831: report/label stable vertical-grid field update should persist the GRIDV field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1831: report/label stable vertical-grid field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1831: label stable vertical-grid field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2025: stable-selected report/label vertical-grid update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1831: report/label stable vertical-grid field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#1831: report/label stable vertical-grid field update should preserve memo-derived top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        "#1831: report/label stable vertical-grid field update should preserve memo-derived bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": " + updated_grid,
                        "#1831: report/label stable vertical-grid field update should refresh vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1831: report/label stable vertical-grid field update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 4",
                        "#1831: report/label stable vertical-grid field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1831: report/label stable vertical-grid field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1831: report/label stable vertical-grid field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_grid + "\""
            },
            "#1831: report/label stable vertical-grid field update should refresh selected direct-field provenance");
    };

    run_grid_vertical_update(temp_root / "grid_vertical_stable.frx",
                             "grid_vertical_stable.frx",
                             "12",
                             "report");
    run_grid_vertical_update(temp_root / "grid_vertical_stable.lbx",
                             "grid_vertical_stable.lbx",
                             "14",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_clears_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_grid_vertical_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_grid_vertical_clear = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_grid_vertical_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "GRIDV",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable vertical-grid field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable vertical-grid field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1831: report/label stable vertical-grid field clear should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "GRIDV"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value.empty(),
               "#1831: report/label stable vertical-grid field clear should blank the GRIDV field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1831: report/label stable vertical-grid field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1831: label stable vertical-grid field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2025: stable-selected report/label vertical-grid clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1831: report/label stable vertical-grid field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#1831: report/label stable vertical-grid field clear should preserve memo-derived top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        "#1831: report/label stable vertical-grid field clear should preserve memo-derived bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1831: report/label stable vertical-grid field clear should clear vertical-grid availability");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 0",
                        "#1831: report/label stable vertical-grid field clear should clear vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        "#1831: report/label stable vertical-grid field clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 3",
                        "#1831: report/label stable vertical-grid field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1831: report/label stable vertical-grid field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1831: report/label stable vertical-grid field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2"
            },
            "#1831: report/label stable vertical-grid field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1831: report/label stable vertical-grid field clear should remove direct GRIDV provenance");
    };

    run_grid_vertical_clear(temp_root / "grid_vertical_clear_stable.frx",
                            "grid_vertical_clear_stable.frx",
                            "report");
    run_grid_vertical_clear(temp_root / "grid_vertical_clear_stable.lbx",
                            "grid_vertical_clear_stable.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_grid_vertical_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_grid_vertical_update = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& updated_grid,
                                                      const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_grid_vertical_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "GRIDV",
                "--property-value", updated_grid,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted vertical-grid field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted vertical-grid field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1831: report/label stable deleted vertical-grid field update should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "GRIDV"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value == updated_grid,
               "#1831: report/label stable deleted vertical-grid field update should persist the GRIDV field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1831: report/label stable deleted vertical-grid field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1831: label stable deleted vertical-grid field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2025: stable-selected deleted report/label vertical-grid update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted vertical-grid field update should expose effective page setup");
        expect_contains(update_process.stdout_text, "\"gridVerticalAvailable\": true",
                        "#3815: report/label stable deleted vertical-grid field update should expose effective availability");
        expect_contains(update_process.stdout_text, "\"gridVertical\": " + updated_grid,
                        "#3815: report/label stable deleted vertical-grid field update should expose the effective value");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1831: report/label stable deleted vertical-grid field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#1831: report/label stable deleted vertical-grid field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1831: report/label stable deleted vertical-grid field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1831: report/label stable deleted vertical-grid field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_grid + "\""
            },
            "#1831: report/label stable deleted vertical-grid field update should refresh selected deleted settings");
    };

    run_deleted_grid_vertical_update(temp_root / "deleted_grid_vertical_stable.frx",
                                     "deleted_grid_vertical_stable.frx",
                                     "12",
                                     "report");
    run_deleted_grid_vertical_update(temp_root / "deleted_grid_vertical_stable.lbx",
                                     "deleted_grid_vertical_stable.lbx",
                                     "14",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_grid_vertical_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_grid_vertical_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_grid_vertical_clear = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_grid_vertical_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "GRIDV",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted vertical-grid field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted vertical-grid field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1831: report/label stable deleted vertical-grid field clear should exit successfully");
        const auto grid_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "GRIDV"
        });
        expect(grid_property.ok && grid_property.exists && grid_property.value.empty(),
               "#1831: report/label stable deleted vertical-grid field clear should blank the GRIDV field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1831: report/label stable deleted vertical-grid field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1831: label stable deleted vertical-grid field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2025: stable-selected deleted report/label vertical-grid clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted vertical-grid field clear should preserve effective page setup");
        expect_contains(clear_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#3815: report/label stable deleted vertical-grid field clear should clear effective availability");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 0",
                        "#3815: report/label stable deleted vertical-grid field clear should reset the effective value");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1831: report/label stable deleted vertical-grid field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1831: report/label stable deleted vertical-grid field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1831: report/label stable deleted vertical-grid field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1831: report/label stable deleted vertical-grid field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2"
            },
            "#1831: report/label stable deleted vertical-grid field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1831: report/label stable deleted vertical-grid field clear should remove direct GRIDV provenance");
    };

    run_deleted_grid_vertical_clear(temp_root / "deleted_grid_vertical_clear_stable.frx",
                                    "deleted_grid_vertical_clear_stable.frx",
                                    "report");
    run_deleted_grid_vertical_clear(temp_root / "deleted_grid_vertical_clear_stable.lbx",
                                    "deleted_grid_vertical_clear_stable.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
