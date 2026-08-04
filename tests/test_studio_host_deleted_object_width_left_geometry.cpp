// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {

void test_studio_host_json_updates_deleted_report_layout_object_width_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_width_update = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1605: deleted report/label layout object width fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "WIDTH",
                "--property-value", "2400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1605: deleted report/label layout object width update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1605: deleted report/label layout object width update should preserve deleted state");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.value == "2400",
               "#1605: deleted report/label layout object width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1605: deleted report/label layout object width update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1605: deleted report/label layout object width update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1605: deleted report/label layout object width update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1605: deleted report/label layout object width update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1605: deleted report/label layout object width update should preserve containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1605: deleted report/label layout object width update should serialize containing-section metadata");
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
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1605: deleted report/label layout object width update should refresh deleted-object geometry metadata");
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
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1605: deleted report/label layout object width update should refresh selected deleted-object geometry metadata");
        expect_not_contains(update_process.stdout_text, "\"width\": 1200",
                            "#1605: deleted report/label layout object width update should not leak stale deleted-object widths");
        expect_not_contains(update_process.stdout_text, "\"right\": 2200",
                            "#1605: deleted report/label layout object width update should not leak stale deleted-object right bounds");
    };

    run_deleted_width_update(temp_root / "deleted_width_update.frx",
                             "deleted_width_update.frx",
                             "report");
    run_deleted_width_update(temp_root / "deleted_width_update.lbx",
                             "deleted_width_update.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_width_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_width_clear = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1606: deleted report/label layout object width clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1606: deleted report/label layout object width clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1606: deleted report/label layout object width clear should preserve deleted state");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1606: deleted report/label layout object width clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1606: deleted report/label layout object width clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1606: deleted report/label layout object width clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1606: deleted report/label layout object width clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1606: deleted report/label layout object width clear should preserve containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1606: deleted report/label layout object width clear should serialize containing-section metadata");
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
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1606: deleted report/label layout object width clear should refresh deleted-object geometry metadata");
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
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1606: deleted report/label layout object width clear should refresh selected deleted-object geometry metadata");
        expect_not_contains(clear_process.stdout_text, "\"width\": 1200",
                            "#1606: deleted report/label layout object width clear should not leak stale deleted-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2200",
                            "#1606: deleted report/label layout object width clear should not leak stale deleted-object right bounds");
    };

    run_deleted_width_clear(temp_root / "deleted_width_clear.frx",
                            "deleted_width_clear.frx",
                            "report");
    run_deleted_width_clear(temp_root / "deleted_width_clear.lbx",
                            "deleted_width_clear.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_width_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_width_update = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1644: deleted report/label layout object stable width fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "WIDTH",
                "--property-value", "2400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1644: deleted report/label layout object stable width update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1644: deleted report/label layout object stable width update should preserve deleted state");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.record_deleted &&
                   width_property.value == "2400",
               "#1644: deleted report/label layout object stable width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1644: deleted report/label layout object stable width update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1644: label deleted layout object stable width update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1644: deleted report/label layout object stable width update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1644: deleted report/label layout object stable width update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1644: deleted report/label layout object stable width update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1644: deleted report/label layout object stable width update should preserve containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1644: deleted report/label layout object stable width update should serialize containing-section metadata");
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
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1644: deleted report/label layout object stable width update should refresh deleted-object geometry metadata");
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
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1644: deleted report/label layout object stable width update should refresh selected deleted-object geometry metadata");
        expect_not_contains(update_process.stdout_text, "\"width\": 1200",
                            "#1644: deleted report/label layout object stable width update should not leak stale deleted-object widths");
        expect_not_contains(update_process.stdout_text, "\"right\": 2200",
                            "#1644: deleted report/label layout object stable width update should not leak stale deleted-object right bounds");
    };

    run_deleted_width_update(temp_root / "deleted_width_update_stable.frx",
                             "deleted_width_update_stable.frx",
                             "report");
    run_deleted_width_update(temp_root / "deleted_width_update_stable.lbx",
                             "deleted_width_update_stable.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_width_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_width_clear = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1645: deleted report/label layout object stable width clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1645: deleted report/label layout object stable width clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1645: deleted report/label layout object stable width clear should preserve deleted state");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.record_deleted &&
                   width_property.direct_field && width_property.value.empty(),
               "#1645: deleted report/label layout object stable width clear should blank the WIDTH field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1645: deleted report/label layout object stable width clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1645: label deleted layout object stable width clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1645: deleted report/label layout object stable width clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1645: deleted report/label layout object stable width clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1645: deleted report/label layout object stable width clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1645: deleted report/label layout object stable width clear should preserve containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1645: deleted report/label layout object stable width clear should serialize containing-section metadata");
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
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1645: deleted report/label layout object stable width clear should refresh deleted-object geometry metadata");
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
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1645: deleted report/label layout object stable width clear should refresh selected deleted-object geometry metadata");
        expect_not_contains(clear_process.stdout_text, "\"width\": 1200",
                            "#1645: deleted report/label layout object stable width clear should not leak stale deleted-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2200",
                            "#1645: deleted report/label layout object stable width clear should not leak stale deleted-object right bounds");
    };

    run_deleted_width_clear(temp_root / "deleted_width_clear_stable.frx",
                            "deleted_width_clear_stable.frx",
                            "report");
    run_deleted_width_clear(temp_root / "deleted_width_clear_stable.lbx",
                            "deleted_width_clear_stable.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_left_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_left_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1607: deleted report/label layout object left update should preserve containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1607: deleted report/label layout object left update should serialize containing-section metadata");
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1608: deleted report/label layout object left clear should preserve containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1608: deleted report/label layout object left clear should serialize containing-section metadata");
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1646: deleted report/label layout object stable left update should preserve containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1646: deleted report/label layout object stable left update should serialize containing-section metadata");
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1647: deleted report/label layout object stable left clear should preserve containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1647: deleted report/label layout object stable left clear should serialize containing-section metadata");
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
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
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

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_deleted_object_width_left_geometry <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_layout_object_width_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_layout_object_width_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_layout_object_width_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_layout_object_width_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_layout_object_left_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_layout_object_left_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_layout_object_left_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_layout_object_left_by_stable_selection(argv[1]);

    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
