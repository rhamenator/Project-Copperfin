// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {

void test_studio_host_json_updates_deleted_report_layout_object_height_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_height_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1609: deleted report/label layout object height update should preserve containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1609: deleted report/label layout object height update should serialize containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1500",
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1500",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1610: deleted report/label layout object height clear should preserve containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1610: deleted report/label layout object height clear should serialize containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 600",
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 600",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1648: deleted report/label layout object stable height update should preserve containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1648: deleted report/label layout object stable height update should serialize containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1500",
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1500",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1649: deleted report/label layout object stable height clear should preserve containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1649: deleted report/label layout object stable height clear should serialize containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 600",
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 600",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1611: deleted report/label layout object top update should preserve containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1611: deleted report/label layout object top update should serialize containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 1400",
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 1400",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1612: deleted report/label layout object top clear should preserve containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1612: deleted report/label layout object top clear should serialize containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"page_header_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 300",
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
                "\"containingSectionId\": \"page_header_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 300",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1650: deleted report/label layout object stable top update should preserve containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1650: deleted report/label layout object stable top update should serialize containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 1400",
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 1400",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1651: deleted report/label layout object stable top clear should preserve containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1651: deleted report/label layout object stable top clear should serialize containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"page_header_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 300",
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
                "\"containingSectionId\": \"page_header_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 300",
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

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_deleted_object_height_top_geometry <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_layout_object_height_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_layout_object_height_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_layout_object_height_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_layout_object_height_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_layout_object_top_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_layout_object_top_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_layout_object_top_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_layout_object_top_by_stable_selection(argv[1]);

    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
