#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_updates_deleted_report_layout_object_left_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_left_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_left_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1607: deleted report/label layout object left fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "HPOS",
                "--property-value", "1400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout left update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout left update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1607: deleted report/label layout object left update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1607: deleted report/label layout object left update should preserve deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "1400",
               "#1607: deleted report/label layout object left update should persist the HPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1607: deleted report/label layout object left update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1607: deleted report/label layout object left update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1607: deleted report/label layout object left update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1607: deleted report/label layout object left update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1607: deleted report/label layout object left update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1607: deleted report/label layout object left update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"width\": 1200",
                "\"right\": 2600"
            },
            "#1607: deleted report/label layout object left update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"width\": 1200",
                "\"right\": 2600"
            },
            "#1607: deleted report/label layout object left update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_left_update(temp_root / "deleted_left_update.frx",
                            "deleted_left_update.frx",
                            "report");
    run_deleted_left_update(temp_root / "deleted_left_update.lbx",
                            "deleted_left_update.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_left_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_left_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_left_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1608: deleted report/label layout object left clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "HPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout left clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout left clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1608: deleted report/label layout object left clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1608: deleted report/label layout object left clear should preserve deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.value.empty(),
               "#1608: deleted report/label layout object left clear should blank the HPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1608: deleted report/label layout object left clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1608: deleted report/label layout object left clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1608: deleted report/label layout object left clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1608: deleted report/label layout object left clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1608: deleted report/label layout object left clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1608: deleted report/label layout object left clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 0",
                "\"width\": 1200",
                "\"right\": 1200"
            },
            "#1608: deleted report/label layout object left clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 0",
                "\"width\": 1200",
                "\"right\": 1200"
            },
            "#1608: deleted report/label layout object left clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_left_clear(temp_root / "deleted_left_clear.frx",
                           "deleted_left_clear.frx",
                           "report");
    run_deleted_left_clear(temp_root / "deleted_left_clear.lbx",
                           "deleted_left_clear.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_left_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_left_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_left_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1646: deleted report/label layout object stable left fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "HPOS",
                "--property-value", "1400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout left update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout left update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1646: deleted report/label layout object stable left update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1646: deleted report/label layout object stable left update should preserve deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.record_deleted &&
                   left_property.value == "1400",
               "#1646: deleted report/label layout object stable left update should persist the HPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1646: deleted report/label layout object stable left update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1646: label deleted layout object stable left update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1646: deleted report/label layout object stable left update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1646: deleted report/label layout object stable left update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1646: deleted report/label layout object stable left update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1646: deleted report/label layout object stable left update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1646: deleted report/label layout object stable left update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"width\": 1200",
                "\"right\": 2600"
            },
            "#1646: deleted report/label layout object stable left update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"width\": 1200",
                "\"right\": 2600"
            },
            "#1646: deleted report/label layout object stable left update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_left_update(temp_root / "deleted_left_update_stable.frx",
                            "deleted_left_update_stable.frx",
                            "report");
    run_deleted_left_update(temp_root / "deleted_left_update_stable.lbx",
                            "deleted_left_update_stable.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_left_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_left_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_left_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1647: deleted report/label layout object stable left clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "HPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout left clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout left clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1647: deleted report/label layout object stable left clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1647: deleted report/label layout object stable left clear should preserve deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.record_deleted &&
                   left_property.direct_field && left_property.value.empty(),
               "#1647: deleted report/label layout object stable left clear should blank the HPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1647: deleted report/label layout object stable left clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1647: label deleted layout object stable left clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1647: deleted report/label layout object stable left clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1647: deleted report/label layout object stable left clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1647: deleted report/label layout object stable left clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1647: deleted report/label layout object stable left clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1647: deleted report/label layout object stable left clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 0",
                "\"width\": 1200",
                "\"right\": 1200"
            },
            "#1647: deleted report/label layout object stable left clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 0",
                "\"width\": 1200",
                "\"right\": 1200"
            },
            "#1647: deleted report/label layout object stable left clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_left_clear(temp_root / "deleted_left_clear_stable.frx",
                           "deleted_left_clear_stable.frx",
                           "report");
    run_deleted_left_clear(temp_root / "deleted_left_clear_stable.lbx",
                           "deleted_left_clear_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_height_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_height_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_height_update = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1609: deleted report/label layout object height fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "HEIGHT",
                "--property-value", "900",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1609: deleted report/label layout object height update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1609: deleted report/label layout object height update should preserve deleted state");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == "900",
               "#1609: deleted report/label layout object height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1609: deleted report/label layout object height update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1609: deleted report/label layout object height update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1609: deleted report/label layout object height update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1609: deleted report/label layout object height update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1609: deleted report/label layout object height update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1609: deleted report/label layout object height update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1609: deleted report/label layout object height update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1609: deleted report/label layout object height update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_height_update(temp_root / "deleted_height_update.frx",
                              "deleted_height_update.frx",
                              "report");
    run_deleted_height_update(temp_root / "deleted_height_update.lbx",
                              "deleted_height_update.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_height_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_height_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_height_clear = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1610: deleted report/label layout object height clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1610: deleted report/label layout object height clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1610: deleted report/label layout object height clear should preserve deleted state");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value.empty(),
               "#1610: deleted report/label layout object height clear should blank the HEIGHT field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1610: deleted report/label layout object height clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1610: deleted report/label layout object height clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1610: deleted report/label layout object height clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1610: deleted report/label layout object height clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1610: deleted report/label layout object height clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1610: deleted report/label layout object height clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 0",
                "\"bottom\": 2600"
            },
            "#1610: deleted report/label layout object height clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 0",
                "\"bottom\": 2600"
            },
            "#1610: deleted report/label layout object height clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_height_clear(temp_root / "deleted_height_clear.frx",
                             "deleted_height_clear.frx",
                             "report");
    run_deleted_height_clear(temp_root / "deleted_height_clear.lbx",
                             "deleted_height_clear.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_height_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_height_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_height_update = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1648: deleted report/label layout object stable height fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "HEIGHT",
                "--property-value", "900",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1648: deleted report/label layout object stable height update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1648: deleted report/label layout object stable height update should preserve deleted state");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.record_deleted &&
                   height_property.value == "900",
               "#1648: deleted report/label layout object stable height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1648: deleted report/label layout object stable height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1648: label deleted layout object stable height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1648: deleted report/label layout object stable height update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1648: deleted report/label layout object stable height update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1648: deleted report/label layout object stable height update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1648: deleted report/label layout object stable height update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1648: deleted report/label layout object stable height update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1648: deleted report/label layout object stable height update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1648: deleted report/label layout object stable height update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_height_update(temp_root / "deleted_height_update_stable.frx",
                              "deleted_height_update_stable.frx",
                              "report");
    run_deleted_height_update(temp_root / "deleted_height_update_stable.lbx",
                              "deleted_height_update_stable.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_height_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_height_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_height_clear = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1649: deleted report/label layout object stable height clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1649: deleted report/label layout object stable height clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1649: deleted report/label layout object stable height clear should preserve deleted state");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.record_deleted &&
                   height_property.direct_field && height_property.value.empty(),
               "#1649: deleted report/label layout object stable height clear should blank the HEIGHT field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1649: deleted report/label layout object stable height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1649: label deleted layout object stable height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1649: deleted report/label layout object stable height clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1649: deleted report/label layout object stable height clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1649: deleted report/label layout object stable height clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1649: deleted report/label layout object stable height clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1649: deleted report/label layout object stable height clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 0",
                "\"bottom\": 2600"
            },
            "#1649: deleted report/label layout object stable height clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 0",
                "\"bottom\": 2600"
            },
            "#1649: deleted report/label layout object stable height clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_height_clear(temp_root / "deleted_height_clear_stable.frx",
                             "deleted_height_clear_stable.frx",
                             "report");
    run_deleted_height_clear(temp_root / "deleted_height_clear_stable.lbx",
                             "deleted_height_clear_stable.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_top_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_top_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_top_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1611: deleted report/label layout object top fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "VPOS",
                "--property-value", "3100",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1611: deleted report/label layout object top update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1611: deleted report/label layout object top update should preserve deleted state");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "3100",
               "#1611: deleted report/label layout object top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1611: deleted report/label layout object top update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1611: deleted report/label layout object top update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1611: deleted report/label layout object top update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1611: deleted report/label layout object top update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1611: deleted report/label layout object top update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1611: deleted report/label layout object top update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 3100",
                "\"height\": 300",
                "\"bottom\": 3400"
            },
            "#1611: deleted report/label layout object top update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 3100",
                "\"height\": 300",
                "\"bottom\": 3400"
            },
            "#1611: deleted report/label layout object top update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_top_update(temp_root / "deleted_top_update.frx",
                           "deleted_top_update.frx",
                           "report");
    run_deleted_top_update(temp_root / "deleted_top_update.lbx",
                           "deleted_top_update.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_top_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_top_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_top_clear = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1612: deleted report/label layout object top clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1612: deleted report/label layout object top clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1612: deleted report/label layout object top clear should preserve deleted state");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value.empty(),
               "#1612: deleted report/label layout object top clear should blank the VPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1612: deleted report/label layout object top clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1612: deleted report/label layout object top clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1612: deleted report/label layout object top clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1612: deleted report/label layout object top clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1612: deleted report/label layout object top clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1612: deleted report/label layout object top clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1612: deleted report/label layout object top clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1612: deleted report/label layout object top clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_top_clear(temp_root / "deleted_top_clear.frx",
                          "deleted_top_clear.frx",
                          "report");
    run_deleted_top_clear(temp_root / "deleted_top_clear.lbx",
                          "deleted_top_clear.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_top_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_top_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_top_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1650: deleted report/label layout object stable top fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "VPOS",
                "--property-value", "3100",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1650: deleted report/label layout object stable top update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1650: deleted report/label layout object stable top update should preserve deleted state");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.record_deleted &&
                   top_property.value == "3100",
               "#1650: deleted report/label layout object stable top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1650: deleted report/label layout object stable top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1650: label deleted layout object stable top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1650: deleted report/label layout object stable top update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1650: deleted report/label layout object stable top update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1650: deleted report/label layout object stable top update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1650: deleted report/label layout object stable top update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1650: deleted report/label layout object stable top update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 3100",
                "\"height\": 300",
                "\"bottom\": 3400"
            },
            "#1650: deleted report/label layout object stable top update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 3100",
                "\"height\": 300",
                "\"bottom\": 3400"
            },
            "#1650: deleted report/label layout object stable top update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_top_update(temp_root / "deleted_top_update_stable.frx",
                           "deleted_top_update_stable.frx",
                           "report");
    run_deleted_top_update(temp_root / "deleted_top_update_stable.lbx",
                           "deleted_top_update_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_top_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_top_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_top_clear = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1651: deleted report/label layout object stable top clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1651: deleted report/label layout object stable top clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1651: deleted report/label layout object stable top clear should preserve deleted state");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.record_deleted &&
                   top_property.direct_field && top_property.value.empty(),
               "#1651: deleted report/label layout object stable top clear should blank the VPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1651: deleted report/label layout object stable top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1651: label deleted layout object stable top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1651: deleted report/label layout object stable top clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1651: deleted report/label layout object stable top clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1651: deleted report/label layout object stable top clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1651: deleted report/label layout object stable top clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1651: deleted report/label layout object stable top clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1651: deleted report/label layout object stable top clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1651: deleted report/label layout object stable top clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_top_clear(temp_root / "deleted_top_clear_stable.frx",
                          "deleted_top_clear_stable.frx",
                          "report");
    run_deleted_top_clear(temp_root / "deleted_top_clear_stable.lbx",
                          "deleted_top_clear_stable.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_edited_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_geometry_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1613: restore edited deleted layout object fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "6",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted layout " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted layout " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1613: deleted report/label layout object geometry pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1613: deleted report/label layout object geometry pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "1400");
        set_deleted_geometry("VPOS", "3100");
        set_deleted_geometry("HEIGHT", "900");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "6",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited deleted layout restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited deleted layout restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1613: edited deleted report/label layout object restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1613: edited deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "1400" &&
                   top_property.ok && top_property.exists && top_property.value == "3100" &&
                   height_property.ok && height_property.exists && height_property.value == "900",
               "#1613: edited deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1613: edited deleted report/label layout object restore should return refreshed report-layout JSON");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1613: edited deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1613: edited deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1613: edited deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1613: edited deleted report/label layout object restore should rehydrate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1613: edited deleted report/label layout object restore should serialize containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 2000",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"top\": 3100",
                "\"width\": 1200",
                "\"height\": 900",
                "\"right\": 2600",
                "\"bottom\": 4000"
            },
            "#1613: edited deleted report/label layout object restore should refresh selected live geometry and section metadata");
    };

    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry.frx",
                                 "deleted_restore_edited_geometry.frx",
                                 "report");
    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry.lbx",
                                 "deleted_restore_edited_geometry.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_edited_geometry_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_geometry_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1652: stable restore edited deleted layout object fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-label-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted layout " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted layout " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1652: stable deleted report/label layout object geometry pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1652: stable deleted report/label layout object geometry pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "1400");
        set_deleted_geometry("VPOS", "3100");
        set_deleted_geometry("HEIGHT", "900");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "deleted-label-guid",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable edited deleted layout restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable edited deleted layout restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1652: stable edited deleted report/label layout object restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1652: stable edited deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && !left_property.record_deleted &&
                   left_property.value == "1400" &&
                   top_property.ok && top_property.exists && !top_property.record_deleted &&
                   top_property.value == "3100" &&
                   height_property.ok && height_property.exists && !height_property.record_deleted &&
                   height_property.value == "900",
               "#1652: stable edited deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1652: stable edited deleted report/label layout object restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1652: label stable edited deleted layout object restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1891: stable edited deleted report/label layout object restore should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1891: stable edited deleted report/label layout object restore should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1652: stable edited deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1652: stable edited deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1652: stable edited deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1652: stable edited deleted report/label layout object restore should rehydrate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1652: stable edited deleted report/label layout object restore should serialize containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 2000",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"top\": 3100",
                "\"width\": 1200",
                "\"height\": 900",
                "\"right\": 2600",
                "\"bottom\": 4000"
            },
            "#1652: stable edited deleted report/label layout object restore should refresh selected live geometry and section metadata");
    };

    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry_stable.frx",
                                 "deleted_restore_edited_geometry_stable.frx",
                                 "report");
    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry_stable.lbx",
                                 "deleted_restore_edited_geometry_stable.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_width_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_width_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_width_update = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "WIDTH",
                "--property-value", "6000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1533: report/label layout object width update should exit successfully");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.value == "6000",
               "#1533: report/label layout object width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1533: report/label layout object width update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1894: label layout object width update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1533: report/label layout object width update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 7200",
                        "#1533: report/label layout object width update should refresh preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 7200",
                        "#1533: report/label layout object width update should refresh preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1894: report/label layout object width update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1894: report/label layout object width update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1894: report/label layout object width update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1894: report/label layout object width update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1894: report/label layout object width update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1894: report/label layout object width update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1894: report/label layout object width update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1533: report/label layout object width update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1533: report/label layout object width update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"width\": 6000",
                "\"right\": 7200",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\""
            },
            "#1533: report/label layout object width update should refresh selected object bounds and preserve section membership");
    };

    run_width_update(temp_root / "width_bounds.frx", "width_bounds.frx", "report");
    run_width_update(temp_root / "width_bounds.lbx", "width_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_width_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_width_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_width_update = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "WIDTH",
                "--property-value", "6000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1636: report/label layout object stable width update should exit successfully");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.value == "6000",
               "#1636: report/label layout object stable width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1636: report/label layout object stable width update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1636: label layout object stable width update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1636: report/label layout object stable width update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 7200",
                        "#1636: report/label layout object stable width update should refresh preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 7200",
                        "#1636: report/label layout object stable width update should refresh preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1895: report/label layout object stable width update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1895: report/label layout object stable width update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1895: report/label layout object stable width update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1895: report/label layout object stable width update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1895: report/label layout object stable width update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1895: report/label layout object stable width update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1895: report/label layout object stable width update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1636: report/label layout object stable width update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1636: report/label layout object stable width update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"width\": 6000",
                "\"right\": 7200",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\""
            },
            "#1636: report/label layout object stable width update should refresh selected object bounds and preserve section membership");
    };

    run_width_update(temp_root / "width_bounds_stable.frx",
                     "width_bounds_stable.frx",
                     "report");
    run_width_update(temp_root / "width_bounds_stable.lbx",
                     "width_bounds_stable.lbx",
                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_width_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_width_clear_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_width_clear = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1560: report/label layout object width clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1560: report/label layout object width clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1896: label layout object width clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1560: report/label layout object width clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1560: report/label layout object width clear should refresh preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1560: report/label layout object width clear should refresh preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1896: report/label layout object width clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1896: report/label layout object width clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1896: report/label layout object width clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1896: report/label layout object width clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1896: report/label layout object width clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1896: report/label layout object width clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1896: report/label layout object width clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1560: report/label layout object width clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1560: report/label layout object width clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"width\": 0",
                "\"right\": 1200",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\""
            },
            "#1560: report/label layout object width clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"width\": 4000",
                            "#1560: report/label layout object width clear should not leak stale selected-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 5200",
                            "#1560: report/label layout object width clear should not leak stale selected-object right bounds");
    };

    run_width_clear(temp_root / "width_clear_bounds.frx", "width_clear_bounds.frx", "report");
    run_width_clear(temp_root / "width_clear_bounds.lbx", "width_clear_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_width_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_width_clear_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_width_clear = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1637: report/label layout object stable width clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1637: report/label layout object stable width clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1637: label layout object stable width clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1637: report/label layout object stable width clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1637: report/label layout object stable width clear should refresh preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1637: report/label layout object stable width clear should refresh preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1897: report/label layout object stable width clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1897: report/label layout object stable width clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1897: report/label layout object stable width clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1897: report/label layout object stable width clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1897: report/label layout object stable width clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1897: report/label layout object stable width clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1897: report/label layout object stable width clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1637: report/label layout object stable width clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1637: report/label layout object stable width clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"width\": 0",
                "\"right\": 1200",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\""
            },
            "#1637: report/label layout object stable width clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"width\": 4000",
                            "#1637: report/label layout object stable width clear should not leak stale selected-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 5200",
                            "#1637: report/label layout object stable width clear should not leak stale selected-object right bounds");
    };

    run_width_clear(temp_root / "width_clear_bounds_stable.frx",
                    "width_clear_bounds_stable.frx",
                    "report");
    run_width_clear(temp_root / "width_clear_bounds_stable.lbx",
                    "width_clear_bounds_stable.lbx",
                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_left_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_left_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_update = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "4",
                "--property-name", "HPOS",
                "--property-value", "-200",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout left update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout left update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1534: report/label layout object left update should exit successfully");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 4U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "-200",
               "#1534: report/label layout object left update should persist the HPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1534: report/label layout object left update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1898: label layout object left update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1534: report/label layout object left update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": -200",
                        "#1534: report/label layout object left update should refresh preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1534: report/label layout object left update should preserve preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 5400",
                        "#1534: report/label layout object left update should refresh preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1898: report/label layout object left update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1898: report/label layout object left update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1898: report/label layout object left update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1898: report/label layout object left update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1898: report/label layout object left update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1898: report/label layout object left update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1898: report/label layout object left update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1534: report/label layout object left update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1534: report/label layout object left update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"containingSectionId\": \"page_header_1\"",
                "\"left\": -200",
                "\"width\": 1800",
                "\"right\": 1600",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"label\""
            },
            "#1534: report/label layout object left update should refresh selected object bounds and preserve section membership");
    };

    run_left_update(temp_root / "left_bounds.frx", "left_bounds.frx", "report");
    run_left_update(temp_root / "left_bounds.lbx", "left_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_left_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_left_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_update = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "label-guid",
                "--property-name", "HPOS",
                "--property-value", "-200",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout left update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout left update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1638: report/label layout object stable left update should exit successfully");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 4U,
            .object_name = {},
            .unique_id = "label-guid",
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "-200",
               "#1638: report/label layout object stable left update should persist the HPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1638: report/label layout object stable left update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1638: label layout object stable left update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1638: report/label layout object stable left update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": -200",
                        "#1638: report/label layout object stable left update should refresh preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1638: report/label layout object stable left update should preserve preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 5400",
                        "#1638: report/label layout object stable left update should refresh preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1899: report/label layout object stable left update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1899: report/label layout object stable left update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1899: report/label layout object stable left update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1899: report/label layout object stable left update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1899: report/label layout object stable left update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1899: report/label layout object stable left update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1899: report/label layout object stable left update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1638: report/label layout object stable left update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1638: report/label layout object stable left update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"containingSectionId\": \"page_header_1\"",
                "\"left\": -200",
                "\"width\": 1800",
                "\"right\": 1600",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"label\""
            },
            "#1638: report/label layout object stable left update should refresh selected object bounds and preserve section membership");
    };

    run_left_update(temp_root / "left_bounds_stable.frx",
                    "left_bounds_stable.frx",
                    "report");
    run_left_update(temp_root / "left_bounds_stable.lbx",
                    "left_bounds_stable.lbx",
                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_left_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_left_clear_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_clear = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "4",
                "--property-name", "HPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout left clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout left clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1561: report/label layout object left clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1561: report/label layout object left clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1900: label layout object left clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1561: report/label layout object left clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1561: report/label layout object left clear should refresh preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1561: report/label layout object left clear should preserve preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1561: report/label layout object left clear should refresh preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1900: report/label layout object left clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1900: report/label layout object left clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1900: report/label layout object left clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1900: report/label layout object left clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1900: report/label layout object left clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1900: report/label layout object left clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1900: report/label layout object left clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1561: report/label layout object left clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1561: report/label layout object left clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"containingSectionId\": \"page_header_1\"",
                "\"left\": 0",
                "\"width\": 1800",
                "\"right\": 1800",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"label\""
            },
            "#1561: report/label layout object left clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"left\": 900",
                            "#1561: report/label layout object left clear should not leak stale selected-object left positions");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2700",
                            "#1561: report/label layout object left clear should not leak stale selected-object right bounds");
    };

    run_left_clear(temp_root / "left_clear_bounds.frx", "left_clear_bounds.frx", "report");
    run_left_clear(temp_root / "left_clear_bounds.lbx", "left_clear_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_left_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_left_clear_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_clear = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "label-guid",
                "--property-name", "HPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout left clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout left clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1639: report/label layout object stable left clear should exit successfully");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 4U,
            .object_name = {},
            .unique_id = "label-guid",
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.direct_field &&
                   left_property.value.empty(),
               "#1639: report/label layout object stable left clear should blank the HPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1639: report/label layout object stable left clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1639: label layout object stable left clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1639: report/label layout object stable left clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1639: report/label layout object stable left clear should refresh preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1639: report/label layout object stable left clear should preserve preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1639: report/label layout object stable left clear should refresh preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1901: report/label layout object stable left clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1901: report/label layout object stable left clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1901: report/label layout object stable left clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1901: report/label layout object stable left clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1901: report/label layout object stable left clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1901: report/label layout object stable left clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1901: report/label layout object stable left clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1639: report/label layout object stable left clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1639: report/label layout object stable left clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"containingSectionId\": \"page_header_1\"",
                "\"left\": 0",
                "\"width\": 1800",
                "\"right\": 1800",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"label\""
            },
            "#1639: report/label layout object stable left clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"left\": 900",
                            "#1639: report/label layout object stable left clear should not leak stale selected-object left positions");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2700",
                            "#1639: report/label layout object stable left clear should not leak stale selected-object right bounds");
    };

    run_left_clear(temp_root / "left_clear_bounds_stable.frx",
                   "left_clear_bounds_stable.frx",
                   "report");
    run_left_clear(temp_root / "left_clear_bounds_stable.lbx",
                   "left_clear_bounds_stable.lbx",
                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_height_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_height_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_height_update = [&](const fs::path& asset_path,
                                       const std::string& title,
                                       const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "HEIGHT",
                "--property-value", "7000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1535: report/label layout object height update should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == "7000",
               "#1535: report/label layout object height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1535: report/label layout object height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1902: label layout object height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1535: report/label layout object height update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 9600",
                        "#1535: report/label layout object height update should refresh preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 9600",
                        "#1535: report/label layout object height update should refresh preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1902: report/label layout object height update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1902: report/label layout object height update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1902: report/label layout object height update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1902: report/label layout object height update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1902: report/label layout object height update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1902: report/label layout object height update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1902: report/label layout object height update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1535: report/label layout object height update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1535: report/label layout object height update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"top\": 2600",
                "\"height\": 7000",
                "\"bottom\": 9600",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 7600",
                "\"objectKind\": \"field\""
            },
            "#1535: report/label layout object height update should refresh selected object bounds and preserve section membership");
    };

    run_height_update(temp_root / "height_bounds.frx", "height_bounds.frx", "report");
    run_height_update(temp_root / "height_bounds.lbx", "height_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_height_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_height_clear_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_height_clear = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1562: report/label layout object height clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1562: report/label layout object height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1904: label layout object height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1562: report/label layout object height clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1562: report/label layout object height clear should preserve document preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1562: report/label layout object height clear should preserve document preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1904: report/label layout object height clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1904: report/label layout object height clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1904: report/label layout object height clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1904: report/label layout object height clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1904: report/label layout object height clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1904: report/label layout object height clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1904: report/label layout object height clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1562: report/label layout object height clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1562: report/label layout object height clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"top\": 2600",
                "\"height\": 0",
                "\"bottom\": 2600",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 600",
                "\"objectKind\": \"field\""
            },
            "#1562: report/label layout object height clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"height\": 450",
                            "#1562: report/label layout object height clear should not leak stale selected-object heights");
        expect_not_contains(clear_process.stdout_text, "\"bottom\": 3050",
                            "#1562: report/label layout object height clear should not leak stale selected-object bottom bounds");
    };

    run_height_clear(temp_root / "height_clear_bounds.frx", "height_clear_bounds.frx", "report");
    run_height_clear(temp_root / "height_clear_bounds.lbx", "height_clear_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_height_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_height_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_height_update = [&](const fs::path& asset_path,
                                       const std::string& title,
                                       const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "HEIGHT",
                "--property-value", "7000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1640: report/label layout object stable height update should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == "7000",
               "#1640: report/label layout object stable height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1640: report/label layout object stable height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1640: label layout object stable height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1640: report/label layout object stable height update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 9600",
                        "#1640: report/label layout object stable height update should refresh preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 9600",
                        "#1640: report/label layout object stable height update should refresh preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1903: report/label layout object stable height update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1903: report/label layout object stable height update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1903: report/label layout object stable height update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1903: report/label layout object stable height update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1903: report/label layout object stable height update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1903: report/label layout object stable height update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1903: report/label layout object stable height update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1640: report/label layout object stable height update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1640: report/label layout object stable height update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"top\": 2600",
                "\"height\": 7000",
                "\"bottom\": 9600",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 7600",
                "\"objectKind\": \"field\""
            },
            "#1640: report/label layout object stable height update should refresh selected object bounds and preserve section membership");
    };

    run_height_update(temp_root / "height_bounds_stable.frx",
                      "height_bounds_stable.frx",
                      "report");
    run_height_update(temp_root / "height_bounds_stable.lbx",
                      "height_bounds_stable.lbx",
                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_height_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_height_clear_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_height_clear = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1641: report/label layout object stable height clear should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.direct_field &&
                   height_property.value.empty(),
               "#1641: report/label layout object stable height clear should blank the HEIGHT field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1641: report/label layout object stable height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1641: label layout object stable height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1641: report/label layout object stable height clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1641: report/label layout object stable height clear should preserve document preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1641: report/label layout object stable height clear should preserve document preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1905: report/label layout object stable height clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1905: report/label layout object stable height clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1905: report/label layout object stable height clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1905: report/label layout object stable height clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1905: report/label layout object stable height clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1905: report/label layout object stable height clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1905: report/label layout object stable height clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1641: report/label layout object stable height clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1641: report/label layout object stable height clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"top\": 2600",
                "\"height\": 0",
                "\"bottom\": 2600",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 600",
                "\"objectKind\": \"field\""
            },
            "#1641: report/label layout object stable height clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"height\": 450",
                            "#1641: report/label layout object stable height clear should not leak stale selected-object heights");
        expect_not_contains(clear_process.stdout_text, "\"bottom\": 3050",
                            "#1641: report/label layout object stable height clear should not leak stale selected-object bottom bounds");
    };

    run_height_clear(temp_root / "height_clear_bounds_stable.frx",
                     "height_clear_bounds_stable.frx",
                     "report");
    run_height_clear(temp_root / "height_clear_bounds_stable.lbx",
                     "height_clear_bounds_stable.lbx",
                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_top_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_top_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_top_update = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "VPOS",
                "--property-value", "-1000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1536: report/label layout object top update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "-1000",
               "#1536: report/label layout object top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1536: report/label layout object top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1906: label layout object top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1536: report/label layout object top update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": -1000",
                        "#1536: report/label layout object top update should refresh preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1536: report/label layout object top update should preserve preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 9100",
                        "#1536: report/label layout object top update should refresh preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1906: report/label layout object top update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1906: report/label layout object top update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1906: report/label layout object top update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1906: report/label layout object top update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1906: report/label layout object top update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1906: report/label layout object top update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1906: report/label layout object top update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1536: report/label layout object top update should decrement placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1536: report/label layout object top update should increment unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1536: report/label layout object top update should clear selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"top\": -1000",
                "\"height\": 450",
                "\"bottom\": -550"
            },
            "#1536: report/label layout object top update should refresh selected object top bounds and unplaced metadata");
    };

    run_top_update(temp_root / "top_bounds.frx", "top_bounds.frx", "report");
    run_top_update(temp_root / "top_bounds.lbx", "top_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_top_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_top_clear_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_top_clear = [&](const fs::path& asset_path,
                                   const std::string& title,
                                   const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1563: report/label layout object top clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1563: report/label layout object top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1908: label layout object top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1563: report/label layout object top clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1563: report/label layout object top clear should preserve document preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1563: report/label layout object top clear should preserve document preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1563: report/label layout object top clear should preserve document preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1908: report/label layout object top clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1908: report/label layout object top clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1908: report/label layout object top clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1908: report/label layout object top clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1908: report/label layout object top clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1908: report/label layout object top clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1908: report/label layout object top clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1563: report/label layout object top clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1563: report/label layout object top clear should preserve unplaced counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1563: report/label layout object top clear should expose selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"page_header_1\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"field\"",
                "\"top\": 0",
                "\"height\": 450",
                "\"bottom\": 450"
            },
            "#1563: report/label layout object top clear should refresh selected object top bounds and section metadata");
    };

    run_top_clear(temp_root / "top_clear_bounds.frx", "top_clear_bounds.frx", "report");
    run_top_clear(temp_root / "top_clear_bounds.lbx", "top_clear_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_top_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_top_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_top_update = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "VPOS",
                "--property-value", "-1000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1642: report/label layout object stable top update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "-1000",
               "#1642: report/label layout object stable top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1642: report/label layout object stable top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1642: label layout object stable top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1642: report/label layout object stable top update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": -1000",
                        "#1642: report/label layout object stable top update should refresh preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1642: report/label layout object stable top update should preserve preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 9100",
                        "#1642: report/label layout object stable top update should refresh preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1907: report/label layout object stable top update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1907: report/label layout object stable top update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1907: report/label layout object stable top update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1907: report/label layout object stable top update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1907: report/label layout object stable top update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1907: report/label layout object stable top update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1907: report/label layout object stable top update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1642: report/label layout object stable top update should decrement placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1642: report/label layout object stable top update should increment unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1642: report/label layout object stable top update should clear selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"top\": -1000",
                "\"height\": 450",
                "\"bottom\": -550"
            },
            "#1642: report/label layout object stable top update should refresh selected object top bounds and unplaced metadata");
    };

    run_top_update(temp_root / "top_bounds_stable.frx",
                   "top_bounds_stable.frx",
                   "report");
    run_top_update(temp_root / "top_bounds_stable.lbx",
                   "top_bounds_stable.lbx",
                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_top_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_top_clear_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_top_clear = [&](const fs::path& asset_path,
                                   const std::string& title,
                                   const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1643: report/label layout object stable top clear should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.direct_field &&
                   top_property.value.empty(),
               "#1643: report/label layout object stable top clear should blank the VPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1643: report/label layout object stable top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1643: label layout object stable top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1643: report/label layout object stable top clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1643: report/label layout object stable top clear should preserve document preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1643: report/label layout object stable top clear should preserve document preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1643: report/label layout object stable top clear should preserve document preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1909: report/label layout object stable top clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1909: report/label layout object stable top clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1909: report/label layout object stable top clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1909: report/label layout object stable top clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1909: report/label layout object stable top clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1909: report/label layout object stable top clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1909: report/label layout object stable top clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1643: report/label layout object stable top clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1643: report/label layout object stable top clear should preserve unplaced counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1643: report/label layout object stable top clear should expose selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"page_header_1\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"field\"",
                "\"top\": 0",
                "\"height\": 450",
                "\"bottom\": 450"
            },
            "#1643: report/label layout object stable top clear should refresh selected object top bounds and section metadata");
    };

    run_top_clear(temp_root / "top_clear_bounds_stable.frx",
                  "top_clear_bounds_stable.frx",
                  "report");
    run_top_clear(temp_root / "top_clear_bounds_stable.lbx",
                  "top_clear_bounds_stable.lbx",
                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_width_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_width_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& updated_width,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_column_width_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "COLWIDTH",
                "--property-value", updated_width,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-width field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-width field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1836: report/label stable column-width field update should exit successfully");
        const auto column_width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "COLWIDTH"
        });
        expect(column_width_property.ok && column_width_property.exists &&
                   column_width_property.value == updated_width,
               "#1836: report/label stable column-width field update should persist the COLWIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1836: report/label stable column-width field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1836: label stable column-width field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2033: stable-selected report/label column-width update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1836: report/label stable column-width field update should not fabricate page setup availability");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1836: report/label stable column-width field update should preserve column setup availability");
        expect_contains(update_process.stdout_text, "\"columnCount\": 2",
                        "#1836: report/label stable column-width field update should preserve memo-derived column counts");
        expect_contains(update_process.stdout_text, "\"columnWidth\": " + updated_width,
                        "#1836: report/label stable column-width field update should refresh column widths");
        expect_contains(update_process.stdout_text, "\"columnSpacing\": 120",
                        "#1836: report/label stable column-width field update should preserve memo-derived column spacing");
        expect_contains(update_process.stdout_text, "\"settingCount\": 3",
                        "#1836: report/label stable column-width field update should preserve setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1836: report/label stable column-width field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1836: report/label stable column-width field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_width + "\""
            },
            "#1836: report/label stable column-width field update should refresh selected direct-field provenance");
    };

    run_column_width_update(temp_root / "column_width_stable.frx",
                            "column_width_stable.frx",
                            "2800",
                            "report");
    run_column_width_update(temp_root / "column_width_stable.lbx",
                            "column_width_stable.lbx",
                            "3000",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_column_width_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_column_width_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_column_width_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "COLWIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable column-width field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable column-width field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1836: report/label stable column-width field clear should exit successfully");
        const auto column_width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "COLWIDTH"
        });
        expect(column_width_property.ok && column_width_property.exists &&
                   column_width_property.value.empty(),
               "#1836: report/label stable column-width field clear should blank the COLWIDTH field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1836: report/label stable column-width field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1836: label stable column-width field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2033: stable-selected report/label column-width clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1836: report/label stable column-width field clear should not fabricate page setup availability");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1836: report/label stable column-width field clear should preserve column setup availability");
        expect_contains(clear_process.stdout_text, "\"columnCount\": 2",
                        "#1836: report/label stable column-width field clear should preserve memo-derived column counts");
        expect_contains(clear_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1836: report/label stable column-width field clear should clear column-width availability");
        expect_contains(clear_process.stdout_text, "\"columnWidth\": 0",
                        "#1836: report/label stable column-width field clear should clear column widths");
        expect_contains(clear_process.stdout_text, "\"columnSpacing\": 120",
                        "#1836: report/label stable column-width field clear should preserve memo-derived column spacing");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 2",
                        "#1836: report/label stable column-width field clear should remove the direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1836: report/label stable column-width field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1836: report/label stable column-width field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1836: report/label stable column-width field clear should preserve remaining selected setting provenance");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1836: report/label stable column-width field clear should remove direct COLWIDTH provenance");
    };

    run_column_width_clear(temp_root / "column_width_clear_stable.frx",
                           "column_width_clear_stable.frx",
                           "report");
    run_column_width_clear(temp_root / "column_width_clear_stable.lbx",
                           "column_width_clear_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_width_field_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_width_update = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& updated_width,
                                                     const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_column_width_field_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "COLWIDTH",
                "--property-value", updated_width,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-width field update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-width field update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1836: report/label stable deleted column-width field update should exit successfully");
        const auto column_width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "COLWIDTH"
        });
        expect(column_width_property.ok && column_width_property.exists &&
                   column_width_property.value == updated_width,
               "#1836: report/label stable deleted column-width field update should persist the COLWIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1836: report/label stable deleted column-width field update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1836: label stable deleted column-width field update should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            update_process.stdout_text,
            "#2033: stable-selected deleted report/label column-width update JSON");
        expect_contains(update_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1836: report/label stable deleted column-width field update should not fabricate live page setup");
        expect_contains(update_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1836: report/label stable deleted column-width field update should not fabricate live column setup");
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        "#1836: report/label stable deleted column-width field update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1836: report/label stable deleted column-width field update should preserve deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1836: report/label stable deleted column-width field update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1836: report/label stable deleted column-width field update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_width + "\""
            },
            "#1836: report/label stable deleted column-width field update should refresh deleted setting provenance");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
                "\"value\": \"" + updated_width + "\""
            },
            "#1836: report/label stable deleted column-width field update should refresh selected deleted settings");
    };

    run_deleted_column_width_update(temp_root / "deleted_column_width_stable.frx",
                                    "deleted_column_width_stable.frx",
                                    "4800",
                                    "report");
    run_deleted_column_width_update(temp_root / "deleted_column_width_stable.lbx",
                                    "deleted_column_width_stable.lbx",
                                    "2400",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_column_width_fields_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_width_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_width_clear = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_column_width_field_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "COLWIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-width field clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-width field clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1836: report/label stable deleted column-width field clear should exit successfully");
        const auto column_width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "deleted-settings-guid",
            .property_name = "COLWIDTH"
        });
        expect(column_width_property.ok && column_width_property.exists &&
                   column_width_property.value.empty(),
               "#1836: report/label stable deleted column-width field clear should blank the COLWIDTH field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1836: report/label stable deleted column-width field clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1836: label stable deleted column-width field clear should retain label identity");
        }
        expect_empty_report_layout_preview_bounds(
            clear_process.stdout_text,
            "#2033: stable-selected deleted report/label column-width clear JSON");
        expect_contains(clear_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1836: report/label stable deleted column-width field clear should not fabricate live page setup");
        expect_contains(clear_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1836: report/label stable deleted column-width field clear should not fabricate live column setup");
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        "#1836: report/label stable deleted column-width field clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 2",
                        "#1836: report/label stable deleted column-width field clear should remove the deleted direct setting from counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1836: report/label stable deleted column-width field clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1836: report/label stable deleted column-width field clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1836: report/label stable deleted column-width field clear should preserve remaining deleted setting provenance");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"COLSPACING\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#1836: report/label stable deleted column-width field clear should preserve remaining selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#1836: report/label stable deleted column-width field clear should remove direct COLWIDTH provenance");
    };

    run_deleted_column_width_clear(temp_root / "deleted_column_width_clear_stable.frx",
                                   "deleted_column_width_clear_stable.frx",
                                   "report");
    run_deleted_column_width_clear(temp_root / "deleted_column_width_clear_stable.lbx",
                                   "deleted_column_width_clear_stable.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_refreshes_deleted_detail_header_footer_section_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_preview_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto update_footer_height_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "700",
                    "--json"
                },
                temp_root);

            if (update_footer_height_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section preview height stdout:\n"
                          << update_footer_height_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section preview height stderr:\n"
                          << update_footer_height_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_height_process.exit_code == 0,
                   "#1820: deleted detail-footer section preview height update by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1820: deleted detail-footer section preview height update should preserve deleted state");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.value == "700",
                   "#1820: deleted detail-footer section preview height update should persist the HEIGHT field");
            expect_contains(update_footer_height_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1820: deleted detail-footer section preview height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_height_process.stdout_text, "\"isLabel\": true",
                                "#1820: deleted detail-footer label section preview height update should retain label identity");
            }
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1820: deleted detail-footer section preview height update should preserve live preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1820: deleted detail-footer section preview height update should preserve live preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve live preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsHeight\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve live preview heights");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1820: deleted detail-footer section preview height update should preserve deleted preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve deleted preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1500",
                            "#1820: deleted detail-footer section preview height update should refresh deleted preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsHeight\": 1000",
                            "#1820: deleted detail-footer section preview height update should refresh deleted preview heights");
            expect_contains(update_footer_height_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve live section height totals");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedSectionHeightTotal\": 1000",
                            "#1820: deleted detail-footer section preview height update should refresh deleted section height totals");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1820: deleted detail-footer section preview height update should preserve selected section availability");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1820: deleted detail-footer section preview height update should preserve selection kind");
            expect_contains_in_order(
                update_footer_height_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"top\": 800",
                    "\"height\": 700",
                    "\"bottom\": 1500"
                },
                "#1820: deleted detail-footer section preview height update should refresh selected deleted-section geometry");

            const auto update_header_top_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "VPOS",
                    "--property-value", "350",
                    "--json"
                },
                temp_root);

            if (update_header_top_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section preview top stdout:\n"
                          << update_header_top_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section preview top stderr:\n"
                          << update_header_top_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_top_process.exit_code == 0,
                   "#1820: deleted detail-header section preview top update by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1820: deleted detail-header section preview top update should preserve deleted state");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.value == "350",
                   "#1820: deleted detail-header section preview top update should persist the VPOS field");
            expect_contains(update_header_top_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1820: deleted detail-header section preview top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_top_process.stdout_text, "\"isLabel\": true",
                                "#1820: deleted detail-header label section preview top update should retain label identity");
            }
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1820: deleted detail-header section preview top update should preserve live preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1820: deleted detail-header section preview top update should preserve live preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1820: deleted detail-header section preview top update should preserve live preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsHeight\": 500",
                            "#1820: deleted detail-header section preview top update should preserve live preview heights");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1820: deleted detail-header section preview top update should preserve deleted preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsTop\": 350",
                            "#1820: deleted detail-header section preview top update should refresh deleted preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1500",
                            "#1820: deleted detail-header section preview top update should preserve expanded deleted preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsHeight\": 1150",
                            "#1820: deleted detail-header section preview top update should refresh deleted preview heights");
            expect_contains(update_header_top_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1820: deleted detail-header section preview top update should preserve live section height totals");
            expect_contains(update_header_top_process.stdout_text, "\"deletedSectionHeightTotal\": 1000",
                            "#1820: deleted detail-header section preview top update should preserve deleted section height totals");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1820: deleted detail-header section preview top update should preserve selected section availability");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1820: deleted detail-header section preview top update should preserve selection kind");
            expect_contains_in_order(
                update_header_top_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"top\": 350",
                    "\"height\": 300",
                    "\"bottom\": 650"
                },
                "#1820: deleted detail-header section preview top update should refresh selected deleted-section geometry");
        };

    run_deleted_detail_header_footer_section_preview_bounds(
        temp_root / "deleted_detail_header_footer_section_preview_bounds_stable.frx",
        "deleted_detail_header_footer_section_preview_bounds_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_preview_bounds(
        temp_root / "deleted_detail_header_footer_section_preview_bounds_stable.lbx",
        "deleted_detail_header_footer_section_preview_bounds_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_refreshes_detail_header_footer_section_delete_restore_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_detail_header_footer_section_delete_restore_preview_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_delete_restore_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto delete_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--delete-object",
                    "--unique-id", "detail-header-guid",
                    "--json"
                },
                temp_root);

            if (delete_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section preview delete stdout:\n"
                          << delete_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section preview delete stderr:\n"
                          << delete_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(delete_process.exit_code == 0,
                   "#2278: detail-header section preview delete by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 0U),
                   "#2278: detail-header section preview delete should mark the section deleted");
            expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#2278: detail-header section preview delete should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                                "#2278: detail-header label section preview delete should retain label identity");
            }
            expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2278: detail-header section preview delete should preserve live preview availability");
            expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 300",
                            "#2278: detail-header section preview delete should shrink live preview top bounds to the sibling section");
            expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2278: detail-header section preview delete should preserve sibling live preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 250",
                            "#2278: detail-header section preview delete should shrink live preview heights to the sibling section");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2278: detail-header section preview delete should expose deleted preview availability");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                            "#2278: detail-header section preview delete should expose deleted preview top bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 300",
                            "#2278: detail-header section preview delete should expose deleted preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                            "#2278: detail-header section preview delete should expose deleted preview heights");
            expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                            "#2278: detail-header section preview delete should keep the sibling section live");
            expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                            "#2278: detail-header section preview delete should expose one deleted section");
            expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 2",
                            "#2278: detail-header section preview delete should keep sibling objects placed");
            expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#2278: detail-header section preview delete should not orphan former header objects");
            expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#2278: detail-header section preview delete should preserve selected section availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#2278: detail-header section preview delete should preserve selection kind");
            expect_contains(delete_process.stdout_text, "\"dryRun\": false",
                            "#2278: detail-header section preview delete JSON should expose committed state");
            expect_contains(delete_process.stdout_text, "\"mutatesAsset\": true",
                            "#2278: detail-header section preview delete JSON should expose mutation state");
            expect_contains(delete_process.stdout_text, "\"undoAvailable\": false",
                            "#2278: detail-header section preview delete JSON should expose undo availability");
            expect_contains(delete_process.stdout_text, "\"undoLabel\": \"\"",
                            "#2278: detail-header section preview delete JSON should expose empty undo labels");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300",
                    "\"objectCount\": 1"
                },
                "#2278: detail-header section preview delete should refresh selected deleted-section geometry");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"objects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#2278: detail-header section preview delete should retain former header object containment inside the deleted section");

            const auto restore_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--restore-object",
                    "--unique-id", "detail-header-guid",
                    "--json"
                },
                temp_root);

            if (restore_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section preview restore stdout:\n"
                          << restore_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section preview restore stderr:\n"
                          << restore_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(restore_process.exit_code == 0,
                   "#2278: detail-header section preview restore by stable selection should exit successfully");
            expect(!dbf_record_deleted(asset_path, 0U),
                   "#2278: detail-header section preview restore should clear the deleted state");
            expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#2278: detail-header section preview restore should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                                "#2278: detail-header label section preview restore should retain label identity");
            }
            expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2278: detail-header section preview restore should preserve live preview availability");
            expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2278: detail-header section preview restore should restore live preview top bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2278: detail-header section preview restore should preserve live preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2278: detail-header section preview restore should restore live preview heights");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2278: detail-header section preview restore should clear deleted preview availability");
            expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                            "#2278: detail-header section preview restore should restore live section counts");
            expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                            "#2278: detail-header section preview restore should clear deleted section counts");
            expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                            "#2278: detail-header section preview restore should restore placed object counts");
            expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#2278: detail-header section preview restore should clear unplaced object counts");
            expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#2278: detail-header section preview restore should preserve selected section availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#2278: detail-header section preview restore should preserve selection kind");
            expect_contains(restore_process.stdout_text, "\"dryRun\": false",
                            "#2278: detail-header section preview restore JSON should expose committed state");
            expect_contains(restore_process.stdout_text, "\"mutatesAsset\": true",
                            "#2278: detail-header section preview restore JSON should expose mutation state");
            expect_contains(restore_process.stdout_text, "\"undoAvailable\": false",
                            "#2278: detail-header section preview restore JSON should expose undo availability");
            expect_contains(restore_process.stdout_text, "\"undoLabel\": \"\"",
                            "#2278: detail-header section preview restore JSON should expose empty undo labels");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"deleted\": false",
                    "\"sectionIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300"
                },
                "#2278: detail-header section preview restore should refresh selected live-section geometry");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"objects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#2278: detail-header section preview restore should restore header object containment");
        };

    const auto run_detail_footer_delete_restore_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto delete_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--delete-object",
                    "--unique-id", "detail-footer-guid",
                    "--json"
                },
                temp_root);

            if (delete_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section preview delete stdout:\n"
                          << delete_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section preview delete stderr:\n"
                          << delete_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(delete_process.exit_code == 0,
                   "#1821: detail-footer section preview delete by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1821: detail-footer section preview delete should mark the section deleted");
            expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1821: detail-footer section preview delete should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                                "#1821: detail-footer label section preview delete should retain label identity");
            }
            expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1821: detail-footer section preview delete should preserve live preview availability");
            expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1821: detail-footer section preview delete should preserve live preview top bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 300",
                            "#1821: detail-footer section preview delete should shrink live preview bottom bounds to the sibling section");
            expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 300",
                            "#1821: detail-footer section preview delete should shrink live preview heights to the sibling section");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1821: detail-footer section preview delete should expose deleted preview availability");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 300",
                            "#1821: detail-footer section preview delete should expose deleted preview top bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 550",
                            "#1821: detail-footer section preview delete should expose deleted preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                            "#1821: detail-footer section preview delete should expose deleted preview heights");
            expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                            "#1821: detail-footer section preview delete should keep the sibling section live");
            expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                            "#1821: detail-footer section preview delete should expose one deleted section");
            expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 2",
                            "#1821: detail-footer section preview delete should keep sibling objects placed");
            expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#1821: detail-footer section preview delete should not orphan former footer objects");
            expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1821: detail-footer section preview delete should preserve selected section availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1821: detail-footer section preview delete should preserve selection kind");
            expect_contains(delete_process.stdout_text, "\"dryRun\": false",
                            "#2241: detail-footer section preview delete JSON should expose committed state");
            expect_contains(delete_process.stdout_text, "\"mutatesAsset\": true",
                            "#2241: detail-footer section preview delete JSON should expose mutation state");
            expect_contains(delete_process.stdout_text, "\"undoAvailable\": false",
                            "#2241: detail-footer section preview delete JSON should expose undo availability");
            expect_contains(delete_process.stdout_text, "\"undoLabel\": \"\"",
                            "#2241: detail-footer section preview delete JSON should expose empty undo labels");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"top\": 300",
                    "\"height\": 250",
                    "\"bottom\": 550",
                    "\"objectCount\": 1"
                },
                "#1821: detail-footer section preview delete should refresh selected deleted-section geometry");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Footer\"",
                    "\"objects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#1821: detail-footer section preview delete should retain former footer object containment inside the deleted section");

            const auto restore_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--restore-object",
                    "--unique-id", "detail-footer-guid",
                    "--json"
                },
                temp_root);

            if (restore_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section preview restore stdout:\n"
                          << restore_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section preview restore stderr:\n"
                          << restore_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(restore_process.exit_code == 0,
                   "#1821: detail-footer section preview restore by stable selection should exit successfully");
            expect(!dbf_record_deleted(asset_path, 2U),
                   "#1821: detail-footer section preview restore should clear the deleted state");
            expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1821: detail-footer section preview restore should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                                "#1821: detail-footer label section preview restore should retain label identity");
            }
            expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1821: detail-footer section preview restore should preserve live preview availability");
            expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1821: detail-footer section preview restore should preserve live preview top bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#1821: detail-footer section preview restore should expand live preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#1821: detail-footer section preview restore should expand live preview heights");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#1821: detail-footer section preview restore should clear deleted preview availability");
            expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                            "#1821: detail-footer section preview restore should restore live section counts");
            expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                            "#1821: detail-footer section preview restore should clear deleted section counts");
            expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                            "#1821: detail-footer section preview restore should restore placed object counts");
            expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#1821: detail-footer section preview restore should clear unplaced object counts");
            expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1821: detail-footer section preview restore should preserve selected section availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1821: detail-footer section preview restore should preserve selection kind");
            expect_contains(restore_process.stdout_text, "\"dryRun\": false",
                            "#2241: detail-footer section preview restore JSON should expose committed state");
            expect_contains(restore_process.stdout_text, "\"mutatesAsset\": true",
                            "#2241: detail-footer section preview restore JSON should expose mutation state");
            expect_contains(restore_process.stdout_text, "\"undoAvailable\": false",
                            "#2241: detail-footer section preview restore JSON should expose undo availability");
            expect_contains(restore_process.stdout_text, "\"undoLabel\": \"\"",
                            "#2241: detail-footer section preview restore JSON should expose empty undo labels");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": false",
                    "\"sectionIndex\": 1",
                    "\"sectionCount\": 2",
                    "\"top\": 300",
                    "\"height\": 250",
                    "\"bottom\": 550"
                },
                "#1821: detail-footer section preview restore should refresh selected live-section geometry");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"objects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#1821: detail-footer section preview restore should restore footer object containment");
        };

    run_detail_header_delete_restore_preview_bounds(
        temp_root / "detail_header_section_delete_restore_preview_bounds.frx",
        "detail_header_section_delete_restore_preview_bounds.frx",
        "report");
    run_detail_header_delete_restore_preview_bounds(
        temp_root / "detail_header_section_delete_restore_preview_bounds.lbx",
        "detail_header_section_delete_restore_preview_bounds.lbx",
        "label");
    run_detail_footer_delete_restore_preview_bounds(
        temp_root / "detail_footer_section_delete_restore_preview_bounds.frx",
        "detail_footer_section_delete_restore_preview_bounds.frx",
        "report");
    run_detail_footer_delete_restore_preview_bounds(
        temp_root / "detail_footer_section_delete_restore_preview_bounds.lbx",
        "detail_footer_section_delete_restore_preview_bounds.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_font_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_fonts =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);

            const auto expect_selected_object_font =
                [&](const std::string& unique_id,
                    const std::string& record_index,
                    const std::string& object_kind,
                    const std::string& containing_section_id,
                    const std::string& containing_section_record_index,
                    const std::string& relative_top,
                    const std::string& relative_bottom,
                    const std::string& expression,
                    const std::string& expression_memo_block,
                    const std::string& fontface,
                    const std::string& fontface_memo_block,
                    const std::string& fontsize,
                    const std::string& mode,
                    const std::string& selection_label) {
                    const auto object_process = run_process_capture(
                        studio_host_path,
                        {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                        temp_root);

                    if (object_process.exit_code != 0) {
                        std::cerr << "studio host " << label << " stable selected " << selection_label
                                  << " font metadata stdout:\n" << object_process.stdout_text << "\n";
                        std::cerr << "studio host " << label << " stable selected " << selection_label
                                  << " font metadata stderr:\n" << object_process.stderr_text << "\n";
                        std::cerr << "fixture root: " << temp_root << "\n";
                    }

                    expect(object_process.exit_code == 0,
                           "#1773: selected detail header/footer object font JSON should exit successfully");
                    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#1773: selected detail header/footer object font JSON should preserve titles");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                                        "#1773: selected detail header/footer label object font JSON should retain identity");
                    }
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                                    "#1773: selected detail header/footer object fonts should select report objects");
                    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                                    "#1773: selected detail header/footer object fonts should expose object selection kind");
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                                    "#1773: selected detail header/footer object fonts should expose containing sections");
                    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                                    "#1773: selected detail header/footer object fonts should not select sections");
                    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1773: selected detail header/footer object fonts should not select settings");
                    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2280: selected detail header/footer object fonts should preserve live preview availability");
                    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2280: selected detail header/footer object fonts should preserve live preview top bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 550",
                                    "#2280: selected detail header/footer object fonts should preserve live preview bottom bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 550",
                                    "#2280: selected detail header/footer object fonts should preserve live preview heights");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                                    "#2280: selected detail header/footer object fonts should not fabricate deleted preview availability");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObject\": {",
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": false",
                            "\"containingSectionId\": \"" + containing_section_id + "\"",
                            "\"containingSectionRecordIndex\": " + containing_section_record_index,
                            "\"sectionRelativeTop\": " + relative_top,
                            "\"sectionRelativeBottom\": " + relative_bottom,
                            "\"sectionObjectIndex\": 0",
                            "\"sectionObjectCount\": 1",
                            "\"objectKind\": \"" + object_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": 2",
                            "\"expressionMemoBlockNumber\": " + expression_memo_block,
                            "\"highlightCount\": 4"
                        },
                        "#1773: stable selected " + selection_label + " should expose selected-object font metadata");
                    expect_contains(object_process.stdout_text,
                                    "\"name\": \"FONTFACE\", \"recordIndex\": " + record_index +
                                        ", \"fieldIndex\": 7, \"sourceLineIndex\": null, \"memoBlockNumber\": " +
                                        fontface_memo_block + ", \"value\": \"" + fontface + "\"",
                                    "#1773: stable selected " + selection_label +
                                        " should expose selected-object FONTFACE provenance");
                    expect_contains(object_process.stdout_text,
                                    "\"name\": \"FONTSIZE\", \"recordIndex\": " + record_index +
                                        ", \"fieldIndex\": 8, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"" +
                                        fontsize + "\"",
                                    "#1773: stable selected " + selection_label +
                                        " should expose selected-object FONTSIZE provenance");
                    expect_contains(object_process.stdout_text,
                                    "\"name\": \"MODE\", \"recordIndex\": " + record_index +
                                        ", \"fieldIndex\": 9, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"" +
                                        mode + "\"",
                                    "#1773: stable selected " + selection_label +
                                        " should expose selected-object MODE provenance");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObjectSection\": {",
                            "\"id\": \"" + containing_section_id + "\"",
                            "\"recordIndex\": " + containing_section_record_index,
                            "\"sectionCount\": 2",
                            "\"objectCount\": 1"
                        },
                        "#1773: stable selected " + selection_label + " should expose containing-section metadata");
                };

            expect_selected_object_font("detail-header-label-guid",
                                        "1",
                                        "label",
                                        "detail-header-guid",
                                        "0",
                                        "50",
                                        "170",
                                        "\\\"Header label\\\"",
                                        "2",
                                        "Courier New",
                                        "3",
                                        "12",
                                        "1",
                                        "detail-header label");
            expect_selected_object_font("detail-footer-field-guid",
                                        "3",
                                        "field",
                                        "detail-footer-guid",
                                        "2",
                                        "60",
                                        "160",
                                        "footer.total",
                                        "5",
                                        "Segoe UI",
                                        "6",
                                        "10",
                                        "2",
                                        "detail-footer field");
        };

    run_detail_header_footer_object_fonts(temp_root / "detail_header_footer_object_fonts.frx",
                                          "detail_header_footer_object_fonts.frx",
                                          "report");
    run_detail_header_footer_object_fonts(temp_root / "detail_header_footer_object_fonts.lbx",
                                          "detail_header_footer_object_fonts.lbx",
                                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_font_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_object_fonts =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1774: detail-header object font fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1774: detail-footer object font fixture should mark the footer object deleted");

            const auto expect_deleted_selected_object_font =
                [&](const std::string& unique_id,
                    const std::string& record_index,
                    const std::string& object_kind,
                    const std::string& containing_section_id,
                    const std::string& containing_section_record_index,
                    const std::string& relative_top,
                    const std::string& relative_bottom,
                    const std::string& expression,
                    const std::string& expression_memo_block,
                    const std::string& fontface,
                    const std::string& fontface_memo_block,
                    const std::string& fontsize,
                    const std::string& mode,
                    const std::string& selection_label) {
                    const auto object_process = run_process_capture(
                        studio_host_path,
                        {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                        temp_root);

                    if (object_process.exit_code != 0) {
                        std::cerr << "studio host " << label << " stable selected deleted " << selection_label
                                  << " font metadata stdout:\n" << object_process.stdout_text << "\n";
                        std::cerr << "studio host " << label << " stable selected deleted " << selection_label
                                  << " font metadata stderr:\n" << object_process.stderr_text << "\n";
                        std::cerr << "fixture root: " << temp_root << "\n";
                    }

                    expect(object_process.exit_code == 0,
                           "#1774: selected deleted detail header/footer object font JSON should exit successfully");
                    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#1774: selected deleted detail header/footer object font JSON should preserve titles");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                                        "#1774: selected deleted detail header/footer label object font JSON should retain identity");
                    }
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                                    "#1774: selected deleted detail header/footer object fonts should select report objects");
                    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                                    "#1774: selected deleted detail header/footer object fonts should advertise report selection");
                    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                                    "#1774: selected deleted detail header/footer object fonts should expose object selection kind");
                    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                                    "#1774: selected deleted detail header/footer object fonts should remove live object counts");
                    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 2",
                                    "#1774: selected deleted detail header/footer object fonts should preserve deleted object counts");
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                                    "#1774: selected deleted detail header/footer object fonts should preserve containing sections");
                    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2281: selected deleted detail header/footer object fonts should preserve live preview availability");
                    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2281: selected deleted detail header/footer object fonts should preserve live preview top bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 550",
                                    "#2281: selected deleted detail header/footer object fonts should preserve live preview bottom bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 550",
                                    "#2281: selected deleted detail header/footer object fonts should preserve live preview heights");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#2281: selected deleted detail header/footer object fonts should expose deleted preview availability");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                                    "#2281: selected deleted detail header/footer object fonts should expose deleted preview top bounds");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                                    "#2281: selected deleted detail header/footer object fonts should expose deleted preview bottom bounds");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                                    "#2281: selected deleted detail header/footer object fonts should expose deleted preview heights");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"deletedObjects\": [",
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": true",
                            "\"containingSectionId\": \"" + containing_section_id + "\"",
                            "\"containingSectionRecordIndex\": " + containing_section_record_index,
                            "\"sectionRelativeTop\": " + relative_top,
                            "\"sectionRelativeBottom\": " + relative_bottom,
                            "\"sectionObjectIndex\": 0",
                            "\"sectionObjectCount\": 1",
                            "\"objectKind\": \"" + object_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": 2",
                            "\"expressionMemoBlockNumber\": " + expression_memo_block,
                            "\"highlightCount\": 4"
                        },
                        "#1774: stable selected deleted " + selection_label +
                            " should expose deleted-object font metadata");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObject\": {",
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": true",
                            "\"containingSectionId\": \"" + containing_section_id + "\"",
                            "\"containingSectionRecordIndex\": " + containing_section_record_index,
                            "\"sectionRelativeTop\": " + relative_top,
                            "\"sectionRelativeBottom\": " + relative_bottom,
                            "\"sectionObjectIndex\": 0",
                            "\"sectionObjectCount\": 1",
                            "\"objectKind\": \"" + object_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": 2",
                            "\"expressionMemoBlockNumber\": " + expression_memo_block,
                            "\"highlightCount\": 4"
                        },
                        "#1774: stable selected deleted " + selection_label +
                            " should expose selected-object font metadata");
                    expect_contains(object_process.stdout_text,
                                    "\"name\": \"FONTFACE\", \"recordIndex\": " + record_index +
                                        ", \"fieldIndex\": 7, \"sourceLineIndex\": null, \"memoBlockNumber\": " +
                                        fontface_memo_block + ", \"value\": \"" + fontface + "\"",
                                    "#1774: stable selected deleted " + selection_label +
                                        " should expose selected-object FONTFACE provenance");
                    expect_contains(object_process.stdout_text,
                                    "\"name\": \"FONTSIZE\", \"recordIndex\": " + record_index +
                                        ", \"fieldIndex\": 8, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"" +
                                        fontsize + "\"",
                                    "#1774: stable selected deleted " + selection_label +
                                        " should expose selected-object FONTSIZE provenance");
                    expect_contains(object_process.stdout_text,
                                    "\"name\": \"MODE\", \"recordIndex\": " + record_index +
                                        ", \"fieldIndex\": 9, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"" +
                                        mode + "\"",
                                    "#1774: stable selected deleted " + selection_label +
                                        " should expose selected-object MODE provenance");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObjectSection\": {",
                            "\"id\": \"" + containing_section_id + "\"",
                            "\"recordIndex\": " + containing_section_record_index,
                            "\"sectionCount\": 2",
                            "\"objectCount\": 0",
                            "\"deletedObjectCount\": 1"
                        },
                        "#1774: stable selected deleted " + selection_label +
                            " should expose containing-section metadata");
                };

            expect_deleted_selected_object_font("detail-header-label-guid",
                                                "1",
                                                "label",
                                                "detail-header-guid",
                                                "0",
                                                "50",
                                                "170",
                                                "\\\"Header label\\\"",
                                                "2",
                                                "Courier New",
                                                "3",
                                                "12",
                                                "1",
                                                "detail-header label");
            expect_deleted_selected_object_font("detail-footer-field-guid",
                                                "3",
                                                "field",
                                                "detail-footer-guid",
                                                "2",
                                                "60",
                                                "160",
                                                "footer.total",
                                                "5",
                                                "Segoe UI",
                                                "6",
                                                "10",
                                                "2",
                                                "detail-footer field");
        };

    run_deleted_detail_header_footer_object_fonts(temp_root / "deleted_detail_header_footer_object_fonts.frx",
                                                  "deleted_detail_header_footer_object_fonts.frx",
                                                  "report");
    run_deleted_detail_header_footer_object_fonts(temp_root / "deleted_detail_header_footer_object_fonts.lbx",
                                                  "deleted_detail_header_footer_object_fonts.lbx",
                                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
