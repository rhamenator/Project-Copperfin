// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
namespace {

void write_stable_summary_object_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "3200", ""},
        {"9", "8", "", "", "3200", "", "700", "summary-section-guid"},
        {"5", "", "\"Summary label\"", "400", "3300", "1500", "250", "summary-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1696: synthetic report table for stable summary object JSON should be created");
}

void write_ambiguous_summary_object_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "3200", ""},
        {"9", "8", "", "", "3200", "", "700", "summary-section-guid"},
        {"5", "", "\"Summary label one\"", "400", "3300", "1500", "250", "duplicate-summary-guid"},
        {"5", "", "\"Summary label two\"", "2100", "3350", "1500", "250", "DUPLICATE-SUMMARY-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1699: synthetic report table for ambiguous stable summary object JSON should be created");
}

void write_live_deleted_ambiguous_summary_object_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "3200", ""},
        {"9", "8", "", "", "3200", "", "700", "summary-section-guid"},
        {"5", "", "\"Live summary label\"", "400", "3300", "1500", "250", "duplicate-live-deleted-guid"},
        {"5", "", "\"Deleted summary label\"", "2100", "3350", "1500", "250", "DUPLICATE-LIVE-DELETED-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1702: synthetic report table for live/deleted ambiguous object JSON should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1702: synthetic report table should mark duplicate object deleted");
}

void write_deep_stable_object_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "3200", ""},
        {"5", "", "\"Preview object 2\"", "100", "200", "1000", "200", ""},
        {"5", "", "\"Preview object 3\"", "100", "500", "1000", "200", ""},
        {"5", "", "\"Preview object 4\"", "100", "800", "1000", "200", ""},
        {"5", "", "\"Preview object 5\"", "100", "1100", "1000", "200", ""},
        {"5", "", "\"Preview object 6\"", "100", "1400", "1000", "200", ""},
        {"5", "", "\"Preview object 7\"", "100", "1700", "1000", "200", ""},
        {"5", "", "\"Preview object 8\"", "100", "2000", "1000", "200", ""},
        {"5", "", "\"Preview object 9\"", "100", "2300", "1000", "200", ""},
        {"5", "", "\"Deep label\"", "400", "2600", "1500", "250", "deep-object-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1705: synthetic report table for deep stable object JSON should be created");
}

void write_live_deleted_ambiguous_summary_section_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", ""},
        {"9", "4", "", "0", "3200", ""},
        {"9", "8", "live summary", "3200", "700", "duplicate-live-deleted-guid"},
        {"9", "8", "deleted summary", "3900", "500", "DUPLICATE-LIVE-DELETED-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1702: synthetic report table for live/deleted ambiguous section JSON should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1702: synthetic report table should mark duplicate section deleted");
}

void write_live_deleted_ambiguous_settings_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "duplicate-live-deleted-guid"},
        {"1", "53", "PAPERSIZE=1", "", "", "DUPLICATE-LIVE-DELETED-GUID"},
        {"9", "4", "", "0", "3200", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1702: synthetic report table for live/deleted ambiguous settings JSON should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1702: synthetic report table should mark duplicate settings deleted");
}

void write_missing_unique_id_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", ""},
        {"9", "4", "", "", "0", "", "3200"},
        {"5", "", "\"No unique id label\"", "400", "1200", "1500", "250"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1711: synthetic report table without UNIQUEID field should be created");
}

}  // namespace

void test_studio_host_json_clears_report_selection_for_missing_stable_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_selection_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_missing_selector = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_stable_summary_object_fixture(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "missing-summary-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing stable report selector stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing stable report selector stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1698: missing stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1698: missing stable report selectors should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1698: missing stable label selectors should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1698: missing stable selectors should not advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1698: missing stable selectors should expose explicit no-selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1698: missing stable selectors should not advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObject\": null",
                        "#1698: missing stable selectors should serialize null selected objects");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1698: missing stable selectors should not advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1698: missing stable selectors should serialize null containing sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1698: missing stable selectors should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1698: missing stable selectors should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1698: missing stable selectors should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1698: missing stable selectors should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 2",
                        "#1698: missing stable selectors should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1698: missing stable selectors should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1698: missing stable selectors should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1698: missing stable selectors should preserve deleted object counts");
    };

    run_missing_selector(temp_root / "missing_summary_selector.frx",
                         "missing_summary_selector.frx",
                         "report");
    run_missing_selector(temp_root / "missing_summary_selector.lbx",
                         "missing_summary_selector.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_selection_for_blank_stable_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_blank_report_selection_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_blank_selector = [&](const fs::path& asset_path,
                                        const std::string& title,
                                        const std::string& label) {
        write_deep_stable_object_fixture(asset_path);

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "   \t  ", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " blank stable selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " blank stable selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1710: blank stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1710: blank stable selectors should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1710: blank stable label selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1710: blank stable selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1710: blank stable selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1710: blank stable selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1710: blank stable selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1710: blank stable selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1710: blank stable selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1710: blank stable selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1710: blank stable selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1710: blank stable selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1710: blank stable selectors should serialize null selected settings");
        expect_not_contains(process.stdout_text, "\"selectedReportObject\": {",
                            "#1710: blank stable selectors should not match blank stored UNIQUEID rows");
    };

    run_blank_selector(temp_root / "blank_selector.frx",
                       "blank_selector.frx",
                       "report");
    run_blank_selector(temp_root / "blank_selector.lbx",
                       "blank_selector.lbx",
                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_selection_when_unique_id_field_is_missing(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_unique_id_report_selection_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_missing_unique_id_field_selector = [&](const fs::path& asset_path,
                                                          const std::string& title,
                                                          const std::string& label) {
        write_missing_unique_id_fixture(asset_path);

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "missing-field-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing UNIQUEID selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing UNIQUEID selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1711: missing UNIQUEID stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1711: missing UNIQUEID stable selectors should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1711: missing UNIQUEID stable label selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1711: missing UNIQUEID stable selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1711: missing UNIQUEID stable selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1711: missing UNIQUEID stable selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1711: missing UNIQUEID stable selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1711: missing UNIQUEID stable selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1711: missing UNIQUEID stable selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1711: missing UNIQUEID stable selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1711: missing UNIQUEID stable selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1711: missing UNIQUEID stable selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1711: missing UNIQUEID stable selectors should serialize null selected settings");
        expect_contains(process.stdout_text, "\"sectionCount\": 1",
                        "#1711: missing UNIQUEID stable selectors should preserve live section counts");
        expect_contains(process.stdout_text, "\"settingCount\": 1",
                        "#1711: missing UNIQUEID stable selectors should preserve live settings counts");
        expect_contains(process.stdout_text, "\"liveObjectCount\": 1",
                        "#1711: missing UNIQUEID stable selectors should preserve live object counts");
    };

    run_missing_unique_id_field_selector(temp_root / "missing_unique_id.frx",
                                         "missing_unique_id.frx",
                                         "report");
    run_missing_unique_id_field_selector(temp_root / "missing_unique_id.lbx",
                                         "missing_unique_id.lbx",
                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_selection_for_ambiguous_stable_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_ambiguous_report_selection_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_ambiguous_selector = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_ambiguous_summary_object_fixture(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-summary-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " ambiguous stable report selector stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " ambiguous stable report selector stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1699: ambiguous stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1699: ambiguous stable report selectors should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1699: ambiguous stable label selectors should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1699: ambiguous stable selectors should not advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1699: ambiguous stable selectors should expose explicit no-selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1699: ambiguous stable selectors should not advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObject\": null",
                        "#1699: ambiguous stable selectors should serialize null selected objects");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1699: ambiguous stable selectors should not advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1699: ambiguous stable selectors should serialize null containing sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1699: ambiguous stable selectors should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1699: ambiguous stable selectors should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1699: ambiguous stable selectors should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1699: ambiguous stable selectors should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 2",
                        "#1699: ambiguous stable selectors should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1699: ambiguous stable selectors should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 2",
                        "#1699: ambiguous stable selectors should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1699: ambiguous stable selectors should preserve deleted object counts");
    };

    run_ambiguous_selector(temp_root / "ambiguous_summary_selector.frx",
                           "ambiguous_summary_selector.frx",
                           "report");
    run_ambiguous_selector(temp_root / "ambiguous_summary_selector.lbx",
                           "ambiguous_summary_selector.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_selection_for_live_deleted_ambiguous_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_live_deleted_ambiguous_report_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_selection = [&](const ProcessResult& process,
                                         const std::string& label,
                                         const std::string& title) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " live/deleted ambiguous report selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " live/deleted ambiguous report selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1702: live/deleted ambiguous stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1702: live/deleted ambiguous stable selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1702: live/deleted ambiguous stable label selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1702: live/deleted ambiguous stable selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1702: live/deleted ambiguous stable selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1702: live/deleted ambiguous stable selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1702: live/deleted ambiguous stable selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1702: live/deleted ambiguous stable selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1702: live/deleted ambiguous stable selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1702: live/deleted ambiguous stable selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1702: live/deleted ambiguous stable selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1702: live/deleted ambiguous stable selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1702: live/deleted ambiguous stable selectors should serialize null selected settings");
    };

    const auto run_live_deleted_ambiguous_object_selector = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_live_deleted_ambiguous_summary_object_fixture(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-live-deleted-guid", "--json"},
            temp_root);

        expect_no_selection(object_process, label, title);
        expect_contains(object_process.stdout_text, "\"sectionCount\": 2",
                        "#1702: live/deleted ambiguous object selectors should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1702: live/deleted ambiguous object selectors should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1702: live/deleted ambiguous object selectors should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1702: live/deleted ambiguous object selectors should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjects\": [",
                        "#1702: live/deleted ambiguous object selectors should preserve deleted object payloads");
    };

    const auto run_live_deleted_ambiguous_section_selector = [&](const fs::path& asset_path,
                                                                 const std::string& title,
                                                                 const std::string& label) {
        write_live_deleted_ambiguous_summary_section_fixture(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-live-deleted-guid", "--json"},
            temp_root);

        expect_no_selection(section_process, label, title);
        expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                        "#1702: live/deleted ambiguous section selectors should preserve live section counts");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1702: live/deleted ambiguous section selectors should preserve deleted section counts");
        expect_contains(section_process.stdout_text, "\"settingCount\": 1",
                        "#1702: live/deleted ambiguous section selectors should preserve live setting counts");
        expect_contains(section_process.stdout_text, "\"deletedSections\": [",
                        "#1702: live/deleted ambiguous section selectors should preserve deleted section payloads");
    };

    const auto run_live_deleted_ambiguous_settings_selector = [&](const fs::path& asset_path,
                                                                  const std::string& title,
                                                                  const std::string& label) {
        write_live_deleted_ambiguous_settings_fixture(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-live-deleted-guid", "--json"},
            temp_root);

        expect_no_selection(settings_process, label, title);
        expect_contains(settings_process.stdout_text, "\"sectionCount\": 1",
                        "#1702: live/deleted ambiguous settings selectors should preserve live section counts");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 1",
                        "#1702: live/deleted ambiguous settings selectors should preserve live setting counts");
        expect_contains(settings_process.stdout_text, "\"deletedSettingCount\": 1",
                        "#1702: live/deleted ambiguous settings selectors should preserve deleted setting counts");
        expect_contains(settings_process.stdout_text, "\"deletedSettings\": [",
                        "#1702: live/deleted ambiguous settings selectors should preserve deleted settings payloads");
        expect_contains(settings_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1702: live/deleted ambiguous settings selectors should preserve live page setup summaries");
    };

    run_live_deleted_ambiguous_object_selector(temp_root / "live_deleted_ambiguous_object.frx",
                                               "live_deleted_ambiguous_object.frx",
                                               "report object");
    run_live_deleted_ambiguous_object_selector(temp_root / "live_deleted_ambiguous_object.lbx",
                                               "live_deleted_ambiguous_object.lbx",
                                               "label object");
    run_live_deleted_ambiguous_section_selector(temp_root / "live_deleted_ambiguous_section.frx",
                                                "live_deleted_ambiguous_section.frx",
                                                "report section");
    run_live_deleted_ambiguous_section_selector(temp_root / "live_deleted_ambiguous_section.lbx",
                                                "live_deleted_ambiguous_section.lbx",
                                                "label section");
    run_live_deleted_ambiguous_settings_selector(temp_root / "live_deleted_ambiguous_settings.frx",
                                                 "live_deleted_ambiguous_settings.frx",
                                                 "report settings");
    run_live_deleted_ambiguous_settings_selector(temp_root / "live_deleted_ambiguous_settings.lbx",
                                                 "live_deleted_ambiguous_settings.lbx",
                                                 "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <studio-host>\n";
        return 1;
    }

    cf_test_studio_host_json::test_studio_host_json_clears_report_selection_for_missing_stable_selector(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_report_selection_for_blank_stable_selector(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_report_selection_when_unique_id_field_is_missing(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_report_selection_for_ambiguous_stable_selector(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_report_selection_for_live_deleted_ambiguous_stable_selectors(argv[1]);
    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
