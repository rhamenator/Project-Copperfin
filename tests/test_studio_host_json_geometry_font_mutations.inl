void test_studio_host_json_updates_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_update = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "FONTFACE",
                "--property-value", "Consolas",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout font update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout font update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1530: report/label layout object font update should exit successfully");
        const auto font_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTFACE"
        });
        expect(font_property.ok && font_property.exists && font_property.value == "Consolas",
               "#1530: report/label layout object font update should persist the FONTFACE memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1530: report/label layout object font update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1530: report/label layout object font update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1530: report/label layout object font update should preserve object selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 2",
                "\"name\": \"EXPR\", \"recordIndex\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"value\": \"Consolas\""
            },
            "#1530: report/label layout object font update should refresh selected object highlight metadata");
    };

    run_font_update(temp_root / "font_update.frx", "font_update.frx", "report");
    run_font_update(temp_root / "font_update.lbx", "font_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_update = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "FONTFACE",
                "--property-value", "Consolas",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout font update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout font update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1632: report/label layout object stable font update should exit successfully");
        const auto font_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "FONTFACE"
        });
        expect(font_property.ok && font_property.exists && font_property.value == "Consolas",
               "#1632: report/label layout object stable font update should persist the FONTFACE memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1632: report/label layout object stable font update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1632: label layout object stable font update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1632: report/label layout object stable font update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1632: report/label layout object stable font update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1632: report/label layout object stable font update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 2",
                "\"name\": \"EXPR\", \"recordIndex\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"value\": \"Consolas\""
            },
            "#1632: report/label layout object stable font update should refresh selected object highlight metadata");
    };

    run_font_update(temp_root / "font_update_stable.frx",
                    "font_update_stable.frx",
                    "report");
    run_font_update(temp_root / "font_update_stable.lbx",
                    "font_update_stable.lbx",
                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_font_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_font_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1603: deleted report/label layout object font update fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "FONTFACE",
                "--property-value", "Consolas",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout font update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout font update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1603: deleted report/label layout object font update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1603: deleted report/label layout object font update should preserve deleted state");
        const auto font_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTFACE"
        });
        expect(font_property.ok && font_property.exists && font_property.value == "Consolas",
               "#1603: deleted report/label layout object font update should persist the FONTFACE memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1603: deleted report/label layout object font update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1603: deleted report/label layout object font update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1603: deleted report/label layout object font update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1603: deleted report/label layout object font update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1603: deleted report/label layout object font update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 2",
                "\"name\": \"EXPR\", \"recordIndex\": 6",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"value\": \"Consolas\""
            },
            "#1603: deleted report/label layout object font update should refresh deleted-object highlight metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 2",
                "\"name\": \"EXPR\", \"recordIndex\": 6",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"value\": \"Consolas\""
            },
            "#1603: deleted report/label layout object font update should refresh selected deleted-object highlight metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#1603: deleted report/label layout object font update should expose live containing-section JSON");
    };

    run_deleted_font_update(temp_root / "deleted_font_update.frx",
                            "deleted_font_update.frx",
                            "report");
    run_deleted_font_update(temp_root / "deleted_font_update.lbx",
                            "deleted_font_update.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_clear = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "FONTFACE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout font clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout font clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1559: report/label layout object font clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1559: report/label layout object font clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1559: report/label layout object font clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1559: report/label layout object font clear should preserve object selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 1",
                "\"name\": \"EXPR\", \"recordIndex\": 3",
                "\"value\": \"customer.company\""
            },
            "#1559: report/label layout object font clear should refresh selected object highlight metadata");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"Segoe UI\"",
                            "#1559: report/label layout object font clear should not leak stale font values");
    };

    run_font_clear(temp_root / "font_clear.frx", "font_clear.frx", "report");
    run_font_clear(temp_root / "font_clear.lbx", "font_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_clear = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "FONTFACE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout font clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout font clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1633: report/label layout object stable font clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1633: report/label layout object stable font clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1633: label layout object stable font clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1633: report/label layout object stable font clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1633: report/label layout object stable font clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1633: report/label layout object stable font clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"sectionRelativeTop\": 600",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 1",
                "\"name\": \"EXPR\", \"recordIndex\": 3",
                "\"value\": \"customer.company\""
            },
            "#1633: report/label layout object stable font clear should refresh selected object highlight metadata");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                            "#1633: report/label layout object stable font clear should remove font highlights");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"Segoe UI\"",
                            "#1633: report/label layout object stable font clear should not leak stale font values");
    };

    run_font_clear(temp_root / "font_clear_stable.frx",
                   "font_clear_stable.frx",
                   "report");
    run_font_clear(temp_root / "font_clear_stable.lbx",
                   "font_clear_stable.lbx",
                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_font_metadata_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_font_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_font_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1604: deleted report/label layout object font clear fixture should start deleted");
        const auto seed_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTFACE",
            .property_value = "Consolas"
        });
        expect(seed_result.ok,
               "#1604: deleted report/label layout object font clear fixture should seed FONTFACE");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1604: deleted report/label layout object font clear seed should preserve deleted state");

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "FONTFACE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout font clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout font clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1604: deleted report/label layout object font clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1604: deleted report/label layout object font clear should preserve deleted state");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1604: deleted report/label layout object font clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1604: deleted report/label layout object font clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1604: deleted report/label layout object font clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1604: deleted report/label layout object font clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1604: deleted report/label layout object font clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 1",
                "\"name\": \"EXPR\", \"recordIndex\": 6",
                "\"value\": \"\\\"Deleted label\\\"\""
            },
            "#1604: deleted report/label layout object font clear should refresh deleted-object highlight metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 1",
                "\"name\": \"EXPR\", \"recordIndex\": 6",
                "\"value\": \"\\\"Deleted label\\\"\""
            },
            "#1604: deleted report/label layout object font clear should refresh selected deleted-object highlight metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#1604: deleted report/label layout object font clear should expose live containing-section JSON");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                            "#1604: deleted report/label layout object font clear should remove deleted font highlights");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"Consolas\"",
                            "#1604: deleted report/label layout object font clear should not leak stale font values");
    };

    run_deleted_font_clear(temp_root / "deleted_font_clear.frx",
                           "deleted_font_clear.frx",
                           "report");
    run_deleted_font_clear(temp_root / "deleted_font_clear.lbx",
                           "deleted_font_clear.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_font_options_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_options_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_option_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_font_options_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "FONTSIZE",
                "--property-value", "14",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout fontsize update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout fontsize update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2691: report/label layout object fontsize update should exit successfully");
        const auto font_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTSIZE"
        });
        expect(font_size_property.ok && font_size_property.exists && font_size_property.value == "14",
               "#2691: report/label layout object fontsize update should persist the FONTSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: report/label layout object fontsize update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2691: label layout object fontsize update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: report/label layout object fontsize update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: report/label layout object fontsize update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: report/label layout object fontsize update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 4",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 3, \"fieldIndex\": 8",
                "\"value\": \"14\"",
                "\"name\": \"MODE\", \"recordIndex\": 3, \"fieldIndex\": 9",
                "\"value\": \"3\""
            },
            "#2691: report/label layout object fontsize update should refresh selected object highlight metadata");

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "MODE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout mode clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout mode clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2691: report/label layout object mode clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: report/label layout object mode clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                        "#2691: report/label layout object mode clear should blank the MODE field");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2691: label layout object mode clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: report/label layout object mode clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: report/label layout object mode clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: report/label layout object mode clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 3, \"fieldIndex\": 8",
                "\"value\": \"14\""
            },
            "#2691: report/label layout object mode clear should refresh selected object highlight metadata");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 3",
                            "#2691: report/label layout object mode clear should remove mode highlights");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"3\"",
                            "#2691: report/label layout object mode clear should not leak stale mode values");
    };

    run_font_option_update(temp_root / "font_options.frx", "font_options.frx", "report");
    run_font_option_update(temp_root / "font_options.lbx", "font_options.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_font_options_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_font_options_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_font_option_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_font_options_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "FONTSIZE",
                "--property-value", "14",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout fontsize update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout fontsize update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2691: report/label layout object stable fontsize update should exit successfully");
        const auto font_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "FONTSIZE"
        });
        expect(font_size_property.ok && font_size_property.exists && font_size_property.value == "14",
               "#2691: report/label layout object stable fontsize update should persist the FONTSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: report/label layout object stable fontsize update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2691: label layout object stable fontsize update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: report/label layout object stable fontsize update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: report/label layout object stable fontsize update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: report/label layout object stable fontsize update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 4",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 3, \"fieldIndex\": 8",
                "\"value\": \"14\"",
                "\"name\": \"MODE\", \"recordIndex\": 3, \"fieldIndex\": 9",
                "\"value\": \"3\""
            },
            "#2691: report/label layout object stable fontsize update should refresh selected object highlight metadata");

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "MODE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout mode clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout mode clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2691: report/label layout object stable mode clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: report/label layout object stable mode clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                        "#2691: report/label layout object stable mode clear should blank the MODE field");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2691: label layout object stable mode clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: report/label layout object stable mode clear should preserve selected object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: report/label layout object stable mode clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: report/label layout object stable mode clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"objectKind\": \"field\"",
                "\"highlightCount\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 3, \"fieldIndex\": 8",
                "\"value\": \"14\""
            },
            "#2691: report/label layout object stable mode clear should refresh selected object highlight metadata");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 3",
                            "#2691: report/label layout object stable mode clear should remove mode highlights");
        expect_not_contains(clear_process.stdout_text, "\"value\": \"3\"",
                            "#2691: report/label layout object stable mode clear should not leak stale mode values");
    };

    run_font_option_update(temp_root / "font_options_stable.frx",
                           "font_options_stable.frx",
                           "report");
    run_font_option_update(temp_root / "font_options_stable.lbx",
                           "font_options_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_font_options_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_font_options_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_font_option_update = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_layout_font_options_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#2691: deleted report/label layout object font option fixture should start deleted");

        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "FONTSIZE",
                "--property-value", "12",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout fontsize update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout fontsize update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2691: deleted report/label layout object fontsize update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#2691: deleted report/label layout object fontsize update should preserve deleted state");
        const auto font_size_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "FONTSIZE"
        });
        expect(font_size_property.ok && font_size_property.exists && font_size_property.value == "12",
               "#2691: deleted report/label layout object fontsize update should persist the FONTSIZE field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: deleted report/label layout object fontsize update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2691: deleted label layout object fontsize update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#2691: deleted report/label layout object fontsize update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: deleted report/label layout object fontsize update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: deleted report/label layout object fontsize update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: deleted report/label layout object fontsize update should preserve containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 4",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 6, \"fieldIndex\": 8",
                "\"value\": \"12\"",
                "\"name\": \"MODE\", \"recordIndex\": 6, \"fieldIndex\": 9",
                "\"value\": \"5\""
            },
            "#2691: deleted report/label layout object fontsize update should refresh deleted-object highlight metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 4",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 6, \"fieldIndex\": 8",
                "\"value\": \"12\"",
                "\"name\": \"MODE\", \"recordIndex\": 6, \"fieldIndex\": 9",
                "\"value\": \"5\""
            },
            "#2691: deleted report/label layout object fontsize update should refresh selected deleted-object highlight metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#2691: deleted report/label layout object fontsize update should expose live containing-section JSON");

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "MODE",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout mode clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout mode clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2691: deleted report/label layout object mode clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#2691: deleted report/label layout object mode clear should preserve deleted state");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2691: deleted report/label layout object mode clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text,
                        "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                        "#2691: deleted report/label layout object mode clear should blank the MODE field");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2691: deleted label layout object mode clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#2691: deleted report/label layout object mode clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#2691: deleted report/label layout object mode clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#2691: deleted report/label layout object mode clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2691: deleted report/label layout object mode clear should preserve containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 6, \"fieldIndex\": 8",
                "\"value\": \"12\""
            },
            "#2691: deleted report/label layout object mode clear should refresh deleted-object highlight metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"label\"",
                "\"highlightCount\": 3",
                "\"name\": \"FONTFACE\", \"recordIndex\": 6",
                "\"name\": \"FONTSIZE\", \"recordIndex\": 6, \"fieldIndex\": 8",
                "\"value\": \"12\""
            },
            "#2691: deleted report/label layout object mode clear should refresh selected deleted-object highlight metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#2691: deleted report/label layout object mode clear should expose live containing-section JSON");
        expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 6",
                            "#2691: deleted report/label layout object mode clear should remove deleted mode highlights");
    };

    run_deleted_font_option_update(temp_root / "deleted_font_options.frx",
                                   "deleted_font_options.frx",
                                   "report");
    run_deleted_font_option_update(temp_root / "deleted_font_options.lbx",
                                   "deleted_font_options.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
