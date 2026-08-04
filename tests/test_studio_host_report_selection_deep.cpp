// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
namespace {

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

void write_deep_stable_section_fixture(const std::filesystem::path& report_path) {
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
        {"9", "8", "deep summary", "", "3200", "", "700", "deep-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1707: synthetic report table for deep stable section JSON should be created");
}

void write_deep_stable_settings_fixture(const std::filesystem::path& report_path) {
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
        {"1", "53", "PAPERSIZE=9", "", "", "", "", "deep-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1707: synthetic report table for deep stable settings JSON should be created");
}

void write_deep_ambiguous_stable_object_fixture(const std::filesystem::path& report_path) {
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
        {"5", "", "\"Preview duplicate\"", "100", "500", "1000", "200", "deep-duplicate-guid"},
        {"5", "", "\"Preview object 4\"", "100", "800", "1000", "200", ""},
        {"5", "", "\"Preview object 5\"", "100", "1100", "1000", "200", ""},
        {"5", "", "\"Preview object 6\"", "100", "1400", "1000", "200", ""},
        {"5", "", "\"Preview object 7\"", "100", "1700", "1000", "200", ""},
        {"5", "", "\"Preview object 8\"", "100", "2000", "1000", "200", ""},
        {"5", "", "\"Preview object 9\"", "100", "2300", "1000", "200", ""},
        {"5", "", "\"Deep duplicate\"", "400", "2600", "1500", "250", "DEEP-DUPLICATE-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1706: synthetic report table for deep ambiguous stable object JSON should be created");
}

void write_deep_live_deleted_ambiguous_object_fixture(const std::filesystem::path& report_path) {
    write_deep_ambiguous_stable_object_fixture(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 10U, true);
    expect(delete_result.ok,
           "#1709: synthetic report table should mark the deep duplicate object deleted");
}

void write_deep_live_deleted_ambiguous_section_fixture(const std::filesystem::path& report_path) {
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
        {"9", "4", "", "", "0", "", "3200", "deep-duplicate-section-guid"},
        {"5", "", "\"Preview object 2\"", "100", "200", "1000", "200", ""},
        {"5", "", "\"Preview object 3\"", "100", "500", "1000", "200", ""},
        {"5", "", "\"Preview object 4\"", "100", "800", "1000", "200", ""},
        {"5", "", "\"Preview object 5\"", "100", "1100", "1000", "200", ""},
        {"5", "", "\"Preview object 6\"", "100", "1400", "1000", "200", ""},
        {"5", "", "\"Preview object 7\"", "100", "1700", "1000", "200", ""},
        {"5", "", "\"Preview object 8\"", "100", "2000", "1000", "200", ""},
        {"5", "", "\"Preview object 9\"", "100", "2300", "1000", "200", ""},
        {"9", "8", "deep summary", "", "3200", "", "700", "DEEP-DUPLICATE-SECTION-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1708: synthetic report table for deep ambiguous stable section JSON should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 10U, true);
    expect(delete_result.ok,
           "#1709: synthetic report table should mark the deep duplicate section deleted");
}

void write_deep_live_deleted_ambiguous_settings_fixture(const std::filesystem::path& report_path) {
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
        {"1", "53", "ORIENTATION=0", "", "", "", "", "deep-duplicate-settings-guid"},
        {"9", "4", "", "", "0", "", "3200", ""},
        {"5", "", "\"Preview object 2\"", "100", "200", "1000", "200", ""},
        {"5", "", "\"Preview object 3\"", "100", "500", "1000", "200", ""},
        {"5", "", "\"Preview object 4\"", "100", "800", "1000", "200", ""},
        {"5", "", "\"Preview object 5\"", "100", "1100", "1000", "200", ""},
        {"5", "", "\"Preview object 6\"", "100", "1400", "1000", "200", ""},
        {"5", "", "\"Preview object 7\"", "100", "1700", "1000", "200", ""},
        {"5", "", "\"Preview object 8\"", "100", "2000", "1000", "200", ""},
        {"5", "", "\"Preview object 9\"", "100", "2300", "1000", "200", ""},
        {"1", "53", "PAPERSIZE=9", "", "", "", "", "DEEP-DUPLICATE-SETTINGS-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1708: synthetic report table for deep ambiguous stable settings JSON should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 10U, true);
    expect(delete_result.ok,
           "#1709: synthetic report table should mark the deep duplicate settings row deleted");
}

}  // namespace

void test_studio_host_json_selects_deep_report_records_by_record_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_report_record_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_common_record_selection = [&](const ProcessResult& process,
                                                    const std::string& label,
                                                    const std::string& title,
                                                    const std::string& selection_kind) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep record selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep record selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1712: deep report/label record selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1712: deep record selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1712: deep record selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1712: deep record selectors should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"" + selection_kind + "\"",
                        "#1712: deep record selectors should expose the selected category");
    };

    const auto run_deep_object_record_selection = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_deep_stable_object_fixture(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "10", "--json"},
            temp_root);

        expect_common_record_selection(object_process, label, title, "object");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1712: deep object record selectors should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1712: deep object record selectors should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1712: deep object record selectors should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 9",
                        "#1712: deep object record selectors should parse beyond the default preview record limit");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 10",
                "\"objectKind\": \"label\"",
                "\"title\": \"\\\"Deep label\\\"\"",
                "\"deleted\": false"
            },
            "#1712: deep object record selectors should expose the selected deep object");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"objectCount\": 9"
            },
            "#1712: deep object record selectors should preserve containing-section metadata");
    };

    const auto run_deep_section_record_selection = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_deep_stable_section_fixture(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "10", "--json"},
            temp_root);

        expect_common_record_selection(section_process, label, title, "section");
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1712: deep section record selectors should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1712: deep section record selectors should not advertise selected-object availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1712: deep section record selectors should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1712: deep section record selectors should parse beyond the default preview record limit");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 10",
                "\"deleted\": false",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 2"
            },
            "#1712: deep section record selectors should expose the selected deep section");
    };

    const auto run_deep_settings_record_selection = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_deep_stable_settings_fixture(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "10", "--json"},
            temp_root);

        expect_common_record_selection(settings_process, label, title, "settings");
        expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1712: deep settings record selectors should advertise selected-settings availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1712: deep settings record selectors should not advertise selected-section availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1712: deep settings record selectors should not advertise selected-object availability");
        expect_contains(settings_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1712: deep settings record selectors should parse beyond the default preview record limit");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1712: deep settings record selectors should preserve all root settings rows");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 10",
                "\"value\": \"9\""
            },
            "#1712: deep settings record selectors should expose the selected deep root settings");
    };

    run_deep_object_record_selection(temp_root / "deep_record_object.frx",
                                     "deep_record_object.frx",
                                     "report object");
    run_deep_object_record_selection(temp_root / "deep_record_object.lbx",
                                     "deep_record_object.lbx",
                                     "label object");
    run_deep_section_record_selection(temp_root / "deep_record_section.frx",
                                      "deep_record_section.frx",
                                      "report section");
    run_deep_section_record_selection(temp_root / "deep_record_section.lbx",
                                      "deep_record_section.lbx",
                                      "label section");
    run_deep_settings_record_selection(temp_root / "deep_record_settings.frx",
                                       "deep_record_settings.frx",
                                       "report settings");
    run_deep_settings_record_selection(temp_root / "deep_record_settings.lbx",
                                       "deep_record_settings.lbx",
                                       "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_selects_deep_report_records_by_stable_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_report_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deep_object_selection = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_deep_stable_object_fixture(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-object-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep stable selector stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep stable selector stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1705: deep stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1705: deep stable selectors should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1705: deep stable label selectors should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1705: deep stable selectors should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1705: deep stable selectors should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1705: deep stable selectors should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1705: deep stable object selectors should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1705: deep stable object selectors should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 9",
                        "#1705: deep stable selectors should parse beyond the default preview record limit");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 10",
                "\"objectKind\": \"label\"",
                "\"title\": \"\\\"Deep label\\\"\"",
                "\"deleted\": false"
            },
            "#1705: deep stable selectors should expose the selected deep report object");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"objectCount\": 9"
            },
            "#1705: deep stable selectors should preserve containing-section metadata for deep objects");
    };

    run_deep_object_selection(temp_root / "deep_selector.frx",
                              "deep_selector.frx",
                              "report");
    run_deep_object_selection(temp_root / "deep_selector.lbx",
                              "deep_selector.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_selection_for_deep_ambiguous_stable_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_ambiguous_report_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deep_ambiguous_selection = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_deep_ambiguous_stable_object_fixture(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep ambiguous stable selector stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep ambiguous stable selector stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1706: deep ambiguous stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1706: deep ambiguous stable selectors should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1706: deep ambiguous stable label selectors should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1706: deep ambiguous stable selectors should not advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1706: deep ambiguous stable selectors should expose explicit no-selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1706: deep ambiguous stable selectors should not advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObject\": null",
                        "#1706: deep ambiguous stable selectors should serialize null selected objects");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1706: deep ambiguous stable selectors should not advertise containing-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1706: deep ambiguous stable selectors should serialize null containing sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1706: deep ambiguous stable selectors should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1706: deep ambiguous stable selectors should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1706: deep ambiguous stable selectors should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1706: deep ambiguous stable selectors should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 1",
                        "#1706: deep ambiguous stable selectors should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1706: deep ambiguous stable selectors should preserve deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 9",
                        "#1706: deep ambiguous stable selectors should parse objects beyond the default preview record limit");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1706: deep ambiguous stable selectors should preserve deleted object counts");
        expect_not_contains(object_process.stdout_text, "\"selectedReportObject\": {",
                            "#1706: deep ambiguous stable selectors should not select the preview-window duplicate");
    };

    run_deep_ambiguous_selection(temp_root / "deep_ambiguous_selector.frx",
                                 "deep_ambiguous_selector.frx",
                                 "report");
    run_deep_ambiguous_selection(temp_root / "deep_ambiguous_selector.lbx",
                                 "deep_ambiguous_selector.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_selection_for_deep_live_deleted_ambiguous_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_live_deleted_ambiguous_report_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_selection = [&](const ProcessResult& process,
                                         const std::string& label,
                                         const std::string& title) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep live/deleted ambiguous stable selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep live/deleted ambiguous stable selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1709: deep live/deleted ambiguous stable selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1709: deep live/deleted ambiguous stable selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1709: deep live/deleted ambiguous stable label selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1709: deep live/deleted ambiguous selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1709: deep live/deleted ambiguous selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1709: deep live/deleted ambiguous selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1709: deep live/deleted ambiguous selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1709: deep live/deleted ambiguous selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1709: deep live/deleted ambiguous selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1709: deep live/deleted ambiguous selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1709: deep live/deleted ambiguous selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1709: deep live/deleted ambiguous selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1709: deep live/deleted ambiguous selectors should serialize null selected settings");
    };

    const auto run_object_selector = [&](const fs::path& asset_path,
                                         const std::string& title,
                                         const std::string& label) {
        write_deep_live_deleted_ambiguous_object_fixture(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-guid", "--json"},
            temp_root);

        expect_no_selection(object_process, label, title);
        expect_contains(object_process.stdout_text, "\"sectionCount\": 1",
                        "#1709: deep live/deleted ambiguous object selectors should preserve live section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1709: deep live/deleted ambiguous object selectors should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1709: deep live/deleted ambiguous object selectors should expose deleted object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjects\": [",
                        "#1709: deep live/deleted ambiguous object selectors should retain deleted-object payloads");
        expect_not_contains(object_process.stdout_text, "\"selectedReportObject\": {",
                            "#1709: deep live/deleted ambiguous object selectors should not select the live preview row");
    };

    const auto run_section_selector = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_deep_live_deleted_ambiguous_section_fixture(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-section-guid", "--json"},
            temp_root);

        expect_no_selection(section_process, label, title);
        expect_contains(section_process.stdout_text, "\"sectionCount\": 1",
                        "#1709: deep live/deleted ambiguous section selectors should preserve live section counts");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1709: deep live/deleted ambiguous section selectors should expose deleted section counts");
        expect_contains(section_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1709: deep live/deleted ambiguous section selectors should parse objects beyond the preview limit");
        expect_contains(section_process.stdout_text, "\"deletedSections\": [",
                        "#1709: deep live/deleted ambiguous section selectors should retain deleted-section payloads");
        expect_not_contains(section_process.stdout_text, "\"selectedReportSection\": {",
                            "#1709: deep live/deleted ambiguous section selectors should not select the live preview row");
    };

    const auto run_settings_selector = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_deep_live_deleted_ambiguous_settings_fixture(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-settings-guid", "--json"},
            temp_root);

        expect_no_selection(settings_process, label, title);
        expect_contains(settings_process.stdout_text, "\"sectionCount\": 1",
                        "#1709: deep live/deleted ambiguous settings selectors should preserve live section counts");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 1",
                        "#1709: deep live/deleted ambiguous settings selectors should preserve live settings counts");
        expect_contains(settings_process.stdout_text, "\"deletedSettingCount\": 1",
                        "#1709: deep live/deleted ambiguous settings selectors should expose deleted settings counts");
        expect_contains(settings_process.stdout_text, "\"deletedSettings\": [",
                        "#1709: deep live/deleted ambiguous settings selectors should retain deleted-settings payloads");
        expect_contains(settings_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1709: deep live/deleted ambiguous settings selectors should parse objects beyond the preview limit");
        expect_not_contains(settings_process.stdout_text, "\"selectedReportSettings\": [",
                            "#1709: deep live/deleted ambiguous settings selectors should not select the live preview row");
    };

    run_object_selector(temp_root / "deep_live_deleted_object.frx",
                        "deep_live_deleted_object.frx",
                        "report object");
    run_object_selector(temp_root / "deep_live_deleted_object.lbx",
                        "deep_live_deleted_object.lbx",
                        "label object");
    run_section_selector(temp_root / "deep_live_deleted_section.frx",
                         "deep_live_deleted_section.frx",
                         "report section");
    run_section_selector(temp_root / "deep_live_deleted_section.lbx",
                         "deep_live_deleted_section.lbx",
                         "label section");
    run_settings_selector(temp_root / "deep_live_deleted_settings.frx",
                          "deep_live_deleted_settings.frx",
                          "report settings");
    run_settings_selector(temp_root / "deep_live_deleted_settings.lbx",
                          "deep_live_deleted_settings.lbx",
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

    cf_test_studio_host_json::test_studio_host_json_selects_deep_report_records_by_record_selector(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_selects_deep_report_records_by_stable_selector(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_report_selection_for_deep_ambiguous_stable_selector(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_report_selection_for_deep_live_deleted_ambiguous_stable_selectors(argv[1]);
    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
