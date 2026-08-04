// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
namespace {
void write_synthetic_report_table_for_local_ambiguous_settings_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "duplicate-settings-guid"},
        {"1", "53", "PAPERSIZE=1", "", "", "DUPLICATE-SETTINGS-GUID"},
        {"9", "4", "", "0", "3200", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1701: synthetic report table for ambiguous stable settings JSON should be created");
}

void write_synthetic_report_table_for_ambiguous_summary_section_json(
    const std::filesystem::path& report_path) {
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
        {"9", "8", "summary one", "3200", "700", "duplicate-section-guid"},
        {"9", "8", "summary two", "3900", "500", "DUPLICATE-SECTION-GUID"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1701: synthetic report table for ambiguous stable summary section JSON should be created");
}
}  // namespace

void test_studio_host_json_clears_report_section_and_settings_selection_for_ambiguous_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_ambiguous_report_selection_category_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto expect_no_selection = [&](const ProcessResult& process,
                                         const std::string& label,
                                         const std::string& title) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " ambiguous report selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " ambiguous report selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1701: ambiguous stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1701: ambiguous stable selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1701: ambiguous stable label selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1701: ambiguous stable selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1701: ambiguous stable selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1701: ambiguous stable selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1701: ambiguous stable selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1701: ambiguous stable selectors should serialize null selected settings");
    };

    const auto run_ambiguous_section_selector = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_ambiguous_summary_section_json(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-section-guid", "--json"},
            temp_root);

        expect_no_selection(section_process, label, title);
        expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                        "#1701: ambiguous section selectors should preserve live section counts");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1701: ambiguous section selectors should preserve deleted section counts");
        expect_contains(section_process.stdout_text, "\"settingCount\": 1",
                        "#1701: ambiguous section selectors should preserve live setting counts");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 3"
            },
            "#1701: ambiguous section selectors should preserve both duplicate section records in layout JSON");
    };

    const auto run_ambiguous_settings_selector = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_local_ambiguous_settings_json(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-settings-guid", "--json"},
            temp_root);

        expect_no_selection(settings_process, label, title);
        expect_contains(settings_process.stdout_text, "\"sectionCount\": 1",
                        "#1701: ambiguous settings selectors should preserve live section counts");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1701: ambiguous settings selectors should preserve both root setting records");
        expect_contains(settings_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1701: ambiguous settings selectors should preserve page setup summaries");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1"
            },
            "#1701: ambiguous settings selectors should preserve duplicate settings records in layout JSON");
    };

    run_ambiguous_section_selector(temp_root / "ambiguous_section_selector.frx",
                                   "ambiguous_section_selector.frx",
                                   "report section");
    run_ambiguous_section_selector(temp_root / "ambiguous_section_selector.lbx",
                                   "ambiguous_section_selector.lbx",
                                   "label section");
    run_ambiguous_settings_selector(temp_root / "ambiguous_settings_selector.frx",
                                    "ambiguous_settings_selector.frx",
                                    "report settings");
    run_ambiguous_settings_selector(temp_root / "ambiguous_settings_selector.lbx",
                                    "ambiguous_settings_selector.lbx",
                                    "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
}  // namespace cf_test_studio_host_json
