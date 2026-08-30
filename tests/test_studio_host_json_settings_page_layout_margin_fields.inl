void test_studio_host_json_updates_report_left_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_left_margin_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_margin_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& updated_margin,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_left_margin_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "LEFTMARGIN",
                "--property-value", updated_margin,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable left margin field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable left margin field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#3015: report/label stable left-margin field update should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "LEFTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value == updated_margin,
               "#3015: report/label stable left-margin field update should persist the LEFTMARGIN field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3015: report/label stable left-margin field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#3015: label stable left-margin field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#3015: stable-selected report/label left-margin update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3015: report/label stable left-margin field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#3015: report/label stable left-margin field update should preserve memo-derived top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        "#3015: report/label stable left-margin field update should preserve memo-derived bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        "#3015: report/label stable left-margin field update should preserve vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        "#3015: report/label stable left-margin field update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 5",
                        "#3015: report/label stable left-margin field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#3015: report/label stable left-margin field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#3015: report/label stable left-margin field update should preserve settings selection kind");
        expect_contains(update_process.stdout_text, "\"leftMarginAvailable\": true",
                        "#3815: report/label stable left-margin field update should expose summary availability");
        expect_contains(update_process.stdout_text, "\"leftMargin\": " + updated_margin,
                        "#3815: report/label stable left-margin field update should expose the summary value");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"LEFTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_margin + "\""
            },
            "#3015: report/label stable left-margin field update should refresh selected direct-field provenance");
    };

    run_left_margin_update(temp_root / "left_margin_stable.frx",
                           "left_margin_stable.frx",
                           "34",
                           "report");
    run_left_margin_update(temp_root / "left_margin_stable.lbx",
                           "left_margin_stable.lbx",
                           "36",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_left_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_left_margin_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_margin_clear = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_stable_left_margin_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "LEFTMARGIN",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable left margin field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable left margin field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#3015: report/label stable left-margin field clear should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "LEFTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value.empty(),
               "#3015: report/label stable left-margin field clear should blank the LEFTMARGIN field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3015: report/label stable left-margin field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#3015: label stable left-margin field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#3015: stable-selected report/label left-margin clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3015: report/label stable left-margin field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#3015: report/label stable left-margin field clear should preserve memo-derived top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        "#3015: report/label stable left-margin field clear should preserve memo-derived bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        "#3015: report/label stable left-margin field clear should preserve vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        "#3015: report/label stable left-margin field clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 4",
                        "#3015: report/label stable left-margin field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#3015: report/label stable left-margin field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#3015: report/label stable left-margin field clear should preserve settings selection kind");
        expect_contains(clear_process.stdout_text, "\"leftMarginAvailable\": false",
                        "#3815: report/label stable left-margin field clear should clear summary availability");
        expect_contains(clear_process.stdout_text, "\"leftMargin\": 0",
                        "#3815: report/label stable left-margin field clear should reset the summary value");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3"
            },
            "#3015: report/label stable left-margin field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"LEFTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#3015: report/label stable left-margin field clear should remove direct LEFTMARGIN provenance");
    };

    run_left_margin_clear(temp_root / "left_margin_clear_stable.frx",
                          "left_margin_clear_stable.frx",
                          "report");
    run_left_margin_clear(temp_root / "left_margin_clear_stable.lbx",
                          "left_margin_clear_stable.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_left_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_left_margin_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_left_margin_update = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& updated_margin,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_left_margin_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "LEFTMARGIN",
                "--property-value", updated_margin,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted left margin field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted left margin field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#3015: report/label stable deleted left-margin field update should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "LEFTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value == updated_margin,
               "#3015: report/label stable deleted left-margin field update should persist the LEFTMARGIN field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3015: report/label stable deleted left-margin field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#3015: label stable deleted left-margin field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#3015: stable-selected deleted report/label left-margin update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted left-margin field update should expose effective page setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#3015: report/label stable deleted left-margin field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 5",
                        "#3015: report/label stable deleted left-margin field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#3015: report/label stable deleted left-margin field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#3015: report/label stable deleted left-margin field update should preserve settings selection kind");
        expect_contains(update_process.stdout_text, "\"leftMarginAvailable\": true",
                        "#3815: report/label stable deleted left-margin field update should expose effective availability");
        expect_contains(update_process.stdout_text, "\"leftMargin\": " + updated_margin,
                        "#3815: report/label stable deleted left-margin field update should expose the effective value");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"LEFTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_margin + "\""
            },
            "#3015: report/label stable deleted left-margin field update should refresh selected deleted settings");
    };

    run_deleted_left_margin_update(temp_root / "deleted_left_margin_stable.frx",
                                   "deleted_left_margin_stable.frx",
                                   "34",
                                   "report");
    run_deleted_left_margin_update(temp_root / "deleted_left_margin_stable.lbx",
                                   "deleted_left_margin_stable.lbx",
                                   "36",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_left_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_left_margin_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_left_margin_clear = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_left_margin_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "LEFTMARGIN",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted left margin field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted left margin field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#3015: report/label stable deleted left-margin field clear should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "LEFTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value.empty(),
               "#3015: report/label stable deleted left-margin field clear should blank the LEFTMARGIN field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3015: report/label stable deleted left-margin field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#3015: label stable deleted left-margin field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#3015: stable-selected deleted report/label left-margin clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted left-margin field clear should preserve effective page setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#3015: report/label stable deleted left-margin field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#3015: report/label stable deleted left-margin field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#3015: report/label stable deleted left-margin field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#3015: report/label stable deleted left-margin field clear should preserve settings selection kind");
        expect_contains(clear_process.stdout_text, "\"leftMarginAvailable\": false",
                        "#3815: report/label stable deleted left-margin field clear should clear effective availability");
        expect_contains(clear_process.stdout_text, "\"leftMargin\": 0",
                        "#3815: report/label stable deleted left-margin field clear should reset the effective value");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3"
            },
            "#3015: report/label stable deleted left-margin field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"LEFTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#3015: report/label stable deleted left-margin field clear should remove direct LEFTMARGIN provenance");
    };

    run_deleted_left_margin_clear(temp_root / "deleted_left_margin_clear_stable.frx",
                                  "deleted_left_margin_clear_stable.frx",
                                  "report");
    run_deleted_left_margin_clear(temp_root / "deleted_left_margin_clear_stable.lbx",
                                  "deleted_left_margin_clear_stable.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_right_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_right_margin_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_right_margin_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& updated_margin,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_right_margin_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "RIGHTMARGIN",
                "--property-value", updated_margin,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable right margin field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable right margin field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#3015: report/label stable right-margin field update should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "RIGHTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value == updated_margin,
               "#3015: report/label stable right-margin field update should persist the RIGHTMARGIN field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3015: report/label stable right-margin field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#3015: label stable right-margin field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#3015: stable-selected report/label right-margin update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3015: report/label stable right-margin field update should preserve page setup availability");
        expect_contains(update_process.stdout_text, "\"topMargin\": 10",
                        "#3015: report/label stable right-margin field update should preserve memo-derived top margins");
        expect_contains(update_process.stdout_text, "\"bottomMargin\": 20",
                        "#3015: report/label stable right-margin field update should preserve memo-derived bottom margins");
        expect_contains(update_process.stdout_text, "\"gridVertical\": 4",
                        "#3015: report/label stable right-margin field update should preserve vertical grid spacing");
        expect_contains(update_process.stdout_text, "\"gridHorizontal\": 8",
                        "#3015: report/label stable right-margin field update should preserve horizontal grid spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 5",
                        "#3015: report/label stable right-margin field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#3015: report/label stable right-margin field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#3015: report/label stable right-margin field update should preserve settings selection kind");
        expect_contains(update_process.stdout_text, "\"rightMarginAvailable\": true",
                        "#3815: report/label stable right-margin field update should expose summary availability");
        expect_contains(update_process.stdout_text, "\"rightMargin\": " + updated_margin,
                        "#3815: report/label stable right-margin field update should expose the summary value");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"RIGHTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_margin + "\""
            },
            "#3015: report/label stable right-margin field update should refresh selected direct-field provenance");
    };

    run_right_margin_update(temp_root / "right_margin_stable.frx",
                            "right_margin_stable.frx",
                            "44",
                            "report");
    run_right_margin_update(temp_root / "right_margin_stable.lbx",
                            "right_margin_stable.lbx",
                            "46",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_right_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_right_margin_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_right_margin_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_right_margin_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "RIGHTMARGIN",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable right margin field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable right margin field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#3015: report/label stable right-margin field clear should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "RIGHTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value.empty(),
               "#3015: report/label stable right-margin field clear should blank the RIGHTMARGIN field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3015: report/label stable right-margin field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#3015: label stable right-margin field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#3015: stable-selected report/label right-margin clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3015: report/label stable right-margin field clear should preserve page setup availability");
        expect_contains(clear_process.stdout_text, "\"topMargin\": 10",
                        "#3015: report/label stable right-margin field clear should preserve memo-derived top margins");
        expect_contains(clear_process.stdout_text, "\"bottomMargin\": 20",
                        "#3015: report/label stable right-margin field clear should preserve memo-derived bottom margins");
        expect_contains(clear_process.stdout_text, "\"gridVertical\": 4",
                        "#3015: report/label stable right-margin field clear should preserve vertical grid spacing");
        expect_contains(clear_process.stdout_text, "\"gridHorizontal\": 8",
                        "#3015: report/label stable right-margin field clear should preserve horizontal grid spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 4",
                        "#3015: report/label stable right-margin field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#3015: report/label stable right-margin field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#3015: report/label stable right-margin field clear should preserve settings selection kind");
        expect_contains(clear_process.stdout_text, "\"rightMarginAvailable\": false",
                        "#3815: report/label stable right-margin field clear should clear summary availability");
        expect_contains(clear_process.stdout_text, "\"rightMargin\": 0",
                        "#3815: report/label stable right-margin field clear should reset the summary value");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3"
            },
            "#3015: report/label stable right-margin field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"RIGHTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#3015: report/label stable right-margin field clear should remove direct RIGHTMARGIN provenance");
    };

    run_right_margin_clear(temp_root / "right_margin_clear_stable.frx",
                           "right_margin_clear_stable.frx",
                           "report");
    run_right_margin_clear(temp_root / "right_margin_clear_stable.lbx",
                           "right_margin_clear_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_right_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_right_margin_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_right_margin_update = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& updated_margin,
                                                     const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_right_margin_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "RIGHTMARGIN",
                "--property-value", updated_margin,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted right margin field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted right margin field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#3015: report/label stable deleted right-margin field update should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "RIGHTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value == updated_margin,
               "#3015: report/label stable deleted right-margin field update should persist the RIGHTMARGIN field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3015: report/label stable deleted right-margin field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#3015: label stable deleted right-margin field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#3015: stable-selected deleted report/label right-margin update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted right-margin field update should expose effective page setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#3015: report/label stable deleted right-margin field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 5",
                        "#3015: report/label stable deleted right-margin field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#3015: report/label stable deleted right-margin field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#3015: report/label stable deleted right-margin field update should preserve settings selection kind");
        expect_contains(update_process.stdout_text, "\"rightMarginAvailable\": true",
                        "#3815: report/label stable deleted right-margin field update should expose effective availability");
        expect_contains(update_process.stdout_text, "\"rightMargin\": " + updated_margin,
                        "#3815: report/label stable deleted right-margin field update should expose the effective value");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"RIGHTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_margin + "\""
            },
            "#3015: report/label stable deleted right-margin field update should refresh selected deleted settings");
    };

    run_deleted_right_margin_update(temp_root / "deleted_right_margin_stable.frx",
                                    "deleted_right_margin_stable.frx",
                                    "44",
                                    "report");
    run_deleted_right_margin_update(temp_root / "deleted_right_margin_stable.lbx",
                                    "deleted_right_margin_stable.lbx",
                                    "46",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_right_margin_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_right_margin_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_right_margin_clear = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_right_margin_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "RIGHTMARGIN",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted right margin field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted right margin field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#3015: report/label stable deleted right-margin field clear should exit successfully");
        const auto margin_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "RIGHTMARGIN"
        });
        expect(margin_property.ok && margin_property.exists && margin_property.value.empty(),
               "#3015: report/label stable deleted right-margin field clear should blank the RIGHTMARGIN field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3015: report/label stable deleted right-margin field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#3015: label stable deleted right-margin field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#3015: stable-selected deleted report/label right-margin clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3815: report/label stable deleted right-margin field clear should preserve effective page setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#3015: report/label stable deleted right-margin field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#3015: report/label stable deleted right-margin field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#3015: report/label stable deleted right-margin field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#3015: report/label stable deleted right-margin field clear should preserve settings selection kind");
        expect_contains(clear_process.stdout_text, "\"rightMarginAvailable\": false",
                        "#3815: report/label stable deleted right-margin field clear should clear effective availability");
        expect_contains(clear_process.stdout_text, "\"rightMargin\": 0",
                        "#3815: report/label stable deleted right-margin field clear should reset the effective value");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3"
            },
            "#3015: report/label stable deleted right-margin field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"RIGHTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#3015: report/label stable deleted right-margin field clear should remove direct RIGHTMARGIN provenance");
    };

    run_deleted_right_margin_clear(temp_root / "deleted_right_margin_clear_stable.frx",
                                   "deleted_right_margin_clear_stable.frx",
                                   "report");
    run_deleted_right_margin_clear(temp_root / "deleted_right_margin_clear_stable.lbx",
                                   "deleted_right_margin_clear_stable.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#include "test_studio_host_json_settings_page_layout_grid_vertical_fields.inl"
