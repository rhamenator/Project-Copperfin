// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_layout_font_options_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "FONTSIZE", .type = 'C', .length = 24U},
        {.name = "MODE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "", "", "10", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "10", "3", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "Tahoma", "11", "5", "", "deleted-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2691: synthetic FRX table for report layout font option JSON should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2691: synthetic FRX table should mark deleted layout objects");
}

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

void test_studio_host_json_updates_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_font_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_font_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "FONTFACE",
                    "--property-value", "Consolas",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object font update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object font update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1775: detail-header object font update should exit successfully");
            const auto updated_font = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "FONTFACE"
            });
            expect(updated_font.ok && updated_font.exists && updated_font.value == "Consolas",
                   "#1775: detail-header object font update should persist the FONTFACE memo field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1775: detail-header object font update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1775: detail-header label object font update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1775: detail-header object font update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1775: detail-header object font update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1775: detail-header object font update should preserve containing-section availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2282: detail-header object font update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2282: detail-header object font update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2282: detail-header object font update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2282: detail-header object font update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2282: detail-header object font update should not fabricate deleted preview availability");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTFACE\", \"recordIndex\": 1",
                    "\"value\": \"Consolas\""
                },
                "#1775: detail-header object font update should refresh selected-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1775: detail-header object font update should preserve containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "FONTFACE",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object font clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object font clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1775: detail-footer object font clear should exit successfully");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"FONTFACE\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 7",
                            "#1775: detail-footer object font clear should blank the FONTFACE memo field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1775: detail-footer object font clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1775: detail-footer label object font clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1775: detail-footer object font clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1775: detail-footer object font clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1775: detail-footer object font clear should preserve containing-section availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2282: detail-footer object font clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2282: detail-footer object font clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2282: detail-footer object font clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2282: detail-footer object font clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2282: detail-footer object font clear should not fabricate deleted preview availability");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1775: detail-footer object font clear should refresh selected-object highlight metadata");
            expect_not_contains(clear_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                                "#1775: detail-footer object font clear should remove stale font highlights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1775: detail-footer object font clear should preserve containing-section metadata");
        };

    run_detail_header_footer_object_font_edits(temp_root / "detail_header_footer_object_font_edits.frx",
                                               "detail_header_footer_object_font_edits.frx",
                                               "report");
    run_detail_header_footer_object_font_edits(temp_root / "detail_header_footer_object_font_edits.lbx",
                                               "detail_header_footer_object_font_edits.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_object_font_metadata_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_object_font_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_font_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1776: deleted detail-header object font fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1776: deleted detail-footer object font fixture should mark the footer object deleted");

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "FONTFACE",
                    "--property-value", "Consolas",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object font update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object font update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1776: deleted detail-header object font update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1776: deleted detail-header object font update should preserve deleted state");
            const auto updated_font = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "FONTFACE"
            });
            expect(updated_font.ok && updated_font.exists && updated_font.value == "Consolas",
                   "#1776: deleted detail-header object font update should persist the FONTFACE memo field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1776: deleted detail-header object font update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1776: deleted detail-header label object font update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1776: deleted detail-header object font update should preserve deleted object counts");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1776: deleted detail-header object font update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1776: deleted detail-header object font update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1776: deleted detail-header object font update should preserve containing sections");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2283: deleted detail-header object font update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2283: deleted detail-header object font update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2283: deleted detail-header object font update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2283: deleted detail-header object font update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2283: deleted detail-header object font update should expose deleted preview availability");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2283: deleted detail-header object font update should preserve deleted preview top bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2283: deleted detail-header object font update should preserve deleted preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2283: deleted detail-header object font update should preserve deleted preview heights");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTFACE\", \"recordIndex\": 1",
                    "\"value\": \"Consolas\""
                },
                "#1776: deleted detail-header object font update should refresh deleted-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTFACE\", \"recordIndex\": 1",
                    "\"value\": \"Consolas\""
                },
                "#1776: deleted detail-header object font update should refresh selected-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"deleted\": false",
                    "\"sectionIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300"
                },
                "#1776: deleted detail-header object font update should expose selected containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "FONTFACE",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object font clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object font clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1776: deleted detail-footer object font clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#1776: deleted detail-footer object font clear should preserve deleted state");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"FONTFACE\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 7",
                            "#1776: deleted detail-footer object font clear should blank the FONTFACE memo field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1776: deleted detail-footer object font clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1776: deleted detail-footer label object font clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1776: deleted detail-footer object font clear should preserve deleted object counts");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1776: deleted detail-footer object font clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1776: deleted detail-footer object font clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1776: deleted detail-footer object font clear should preserve containing sections");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2283: deleted detail-footer object font clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2283: deleted detail-footer object font clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2283: deleted detail-footer object font clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2283: deleted detail-footer object font clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2283: deleted detail-footer object font clear should expose deleted preview availability");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2283: deleted detail-footer object font clear should preserve deleted preview top bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2283: deleted detail-footer object font clear should preserve deleted preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2283: deleted detail-footer object font clear should preserve deleted preview heights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1776: deleted detail-footer object font clear should refresh deleted-object highlight metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1776: deleted detail-footer object font clear should refresh selected-object highlight metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": false",
                    "\"sectionIndex\": 1",
                    "\"sectionCount\": 2",
                    "\"top\": 300",
                    "\"height\": 250",
                    "\"bottom\": 550"
                },
                "#1776: deleted detail-footer object font clear should expose selected containing-section metadata");
            expect_not_contains(clear_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 3",
                                "#1776: deleted detail-footer object font clear should remove stale font highlights");
        };

    run_deleted_detail_header_footer_object_font_edits(
        temp_root / "deleted_detail_header_footer_object_font_edits.frx",
        "deleted_detail_header_footer_object_font_edits.frx",
        "report");
    run_deleted_detail_header_footer_object_font_edits(
        temp_root / "deleted_detail_header_footer_object_font_edits.lbx",
        "deleted_detail_header_footer_object_font_edits.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_detail_header_footer_object_font_options_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_font_option_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_font_options =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "FONTSIZE",
                    "--property-value", "14",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object fontsize update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object fontsize update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1777: detail-header object fontsize update should exit successfully");
            const auto updated_fontsize = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "FONTSIZE"
            });
            expect(updated_fontsize.ok && updated_fontsize.exists && updated_fontsize.value == "14",
                   "#1777: detail-header object fontsize update should persist the FONTSIZE field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1777: detail-header object fontsize update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1777: detail-header label object fontsize update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1777: detail-header object fontsize update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1777: detail-header object fontsize update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1777: detail-header object fontsize update should preserve containing-section availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2284: detail-header object fontsize update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2284: detail-header object fontsize update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2284: detail-header object fontsize update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2284: detail-header object fontsize update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2284: detail-header object fontsize update should not fabricate deleted preview availability");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTSIZE\", \"recordIndex\": 1",
                    "\"value\": \"14\""
                },
                "#1777: detail-header object fontsize update should refresh selected-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1777: detail-header object fontsize update should preserve containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "MODE",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object mode clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object mode clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1777: detail-footer object mode clear should exit successfully");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                            "#1777: detail-footer object mode clear should blank the MODE field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1777: detail-footer object mode clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1777: detail-footer label object mode clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1777: detail-footer object mode clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1777: detail-footer object mode clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1777: detail-footer object mode clear should preserve containing-section availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2284: detail-footer object mode clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2284: detail-footer object mode clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2284: detail-footer object mode clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2284: detail-footer object mode clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2284: detail-footer object mode clear should not fabricate deleted preview availability");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1777: detail-footer object mode clear should refresh selected-object highlight metadata");
            expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 3",
                                "#1777: detail-footer object mode clear should remove stale mode highlights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1777: detail-footer object mode clear should preserve containing-section metadata");
        };

    run_detail_header_footer_object_font_options(temp_root / "detail_header_footer_object_font_options.frx",
                                                 "detail_header_footer_object_font_options.frx",
                                                 "report");
    run_detail_header_footer_object_font_options(temp_root / "detail_header_footer_object_font_options.lbx",
                                                 "detail_header_footer_object_font_options.lbx",
                                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_object_font_options_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_object_font_option_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_font_options =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_font_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1778: deleted detail-header object font option fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1778: deleted detail-footer object font option fixture should mark the footer object deleted");

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "FONTSIZE",
                    "--property-value", "14",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object fontsize update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object fontsize update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1778: deleted detail-header object fontsize update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1778: deleted detail-header object fontsize update should preserve deleted state");
            const auto updated_fontsize = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "FONTSIZE"
            });
            expect(updated_fontsize.ok && updated_fontsize.exists && updated_fontsize.value == "14",
                   "#1778: deleted detail-header object fontsize update should persist the FONTSIZE field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1778: deleted detail-header object fontsize update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1778: deleted detail-header label object fontsize update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1778: deleted detail-header object fontsize update should preserve deleted object counts");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1778: deleted detail-header object fontsize update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1778: deleted detail-header object fontsize update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1778: deleted detail-header object fontsize update should preserve containing sections");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1778: deleted detail-header object fontsize update should serialize containing-section JSON");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2285: deleted detail-header object fontsize update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2285: deleted detail-header object fontsize update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2285: deleted detail-header object fontsize update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2285: deleted detail-header object fontsize update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2285: deleted detail-header object fontsize update should expose deleted preview availability");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2285: deleted detail-header object fontsize update should preserve deleted preview top bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2285: deleted detail-header object fontsize update should preserve deleted preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2285: deleted detail-header object fontsize update should preserve deleted preview heights");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTSIZE\", \"recordIndex\": 1",
                    "\"value\": \"14\""
                },
                "#1778: deleted detail-header object fontsize update should refresh deleted-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"highlightCount\": 4",
                    "\"name\": \"FONTSIZE\", \"recordIndex\": 1",
                    "\"value\": \"14\""
                },
                "#1778: deleted detail-header object fontsize update should refresh selected-object highlight metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1778: deleted detail-header object fontsize update should preserve deleted containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "MODE",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object mode clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object mode clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1778: deleted detail-footer object mode clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#1778: deleted detail-footer object mode clear should preserve deleted state");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"MODE\", \"type\": \"C\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 9",
                            "#1778: deleted detail-footer object mode clear should blank the MODE field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1778: deleted detail-footer object mode clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1778: deleted detail-footer label object mode clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1778: deleted detail-footer object mode clear should preserve deleted object counts");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1778: deleted detail-footer object mode clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1778: deleted detail-footer object mode clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1778: deleted detail-footer object mode clear should preserve containing sections");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1778: deleted detail-footer object mode clear should serialize containing-section JSON");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2285: deleted detail-footer object mode clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2285: deleted detail-footer object mode clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2285: deleted detail-footer object mode clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2285: deleted detail-footer object mode clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2285: deleted detail-footer object mode clear should expose deleted preview availability");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2285: deleted detail-footer object mode clear should preserve deleted preview top bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2285: deleted detail-footer object mode clear should preserve deleted preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2285: deleted detail-footer object mode clear should preserve deleted preview heights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1778: deleted detail-footer object mode clear should refresh deleted-object highlight metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"highlightCount\": 3"
                },
                "#1778: deleted detail-footer object mode clear should refresh selected-object highlight metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1778: deleted detail-footer object mode clear should preserve deleted containing-section metadata");
            expect_not_contains(clear_process.stdout_text, "\"name\": \"MODE\", \"recordIndex\": 3",
                                "#1778: deleted detail-footer object mode clear should remove stale mode highlights");
        };

    run_deleted_detail_header_footer_object_font_options(
        temp_root / "deleted_detail_header_footer_object_font_options.frx",
        "deleted_detail_header_footer_object_font_options.frx",
        "report");
    run_deleted_detail_header_footer_object_font_options(
        temp_root / "deleted_detail_header_footer_object_font_options.lbx",
        "deleted_detail_header_footer_object_font_options.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
