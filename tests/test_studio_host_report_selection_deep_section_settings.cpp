// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
namespace {

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

void write_deep_ambiguous_section_fixture(const std::filesystem::path& report_path) {
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

void write_deep_ambiguous_settings_fixture(const std::filesystem::path& report_path) {
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
}

}  // namespace

void test_studio_host_json_selects_deep_report_sections_and_settings_by_stable_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_report_section_settings_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto expect_common_selection = [&](const ProcessResult& process,
                                             const std::string& label,
                                             const std::string& title,
                                             const std::string& selection_kind) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep stable selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep stable selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1707: deep stable report/label section/settings selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1707: deep stable section/settings selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1707: deep stable label section/settings selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1707: deep stable section/settings selectors should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"" + selection_kind + "\"",
                        "#1707: deep stable section/settings selectors should expose the selected category");
    };

    const auto run_deep_section_selection = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_deep_stable_section_fixture(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-section-guid", "--json"},
            temp_root);

        expect_common_selection(section_process, label, title, "section");
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1707: deep stable section selectors should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1707: deep stable section selectors should not advertise selected-object availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                        "#1707: deep stable section selectors should serialize null selected objects");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1707: deep stable section selectors should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1707: deep stable section selectors should serialize null selected settings");
        expect_contains(section_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1707: deep stable section selectors should parse objects beyond the default preview record limit");
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
            "#1707: deep stable section selectors should expose the selected deep report section");
    };

    const auto run_deep_settings_selection = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_deep_stable_settings_fixture(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-settings-guid", "--json"},
            temp_root);

        expect_common_selection(settings_process, label, title, "settings");
        expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1707: deep stable settings selectors should advertise selected-settings availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1707: deep stable settings selectors should not advertise selected-section availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportSection\": null",
                        "#1707: deep stable settings selectors should serialize null selected sections");
        expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1707: deep stable settings selectors should not advertise selected-object availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportObject\": null",
                        "#1707: deep stable settings selectors should serialize null selected objects");
        expect_contains(settings_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1707: deep stable settings selectors should parse objects beyond the default preview record limit");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1707: deep stable settings selectors should preserve all root settings rows");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 10",
                "\"value\": \"9\""
            },
            "#1707: deep stable settings selectors should expose the selected deep root settings");
    };

    run_deep_section_selection(temp_root / "deep_section_selector.frx",
                               "deep_section_selector.frx",
                               "report section");
    run_deep_section_selection(temp_root / "deep_section_selector.lbx",
                               "deep_section_selector.lbx",
                               "label section");
    run_deep_settings_selection(temp_root / "deep_settings_selector.frx",
                                "deep_settings_selector.frx",
                                "report settings");
    run_deep_settings_selection(temp_root / "deep_settings_selector.lbx",
                                "deep_settings_selector.lbx",
                                "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_selection_for_deep_ambiguous_section_and_settings_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_ambiguous_report_section_settings_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto expect_no_selection = [&](const ProcessResult& process,
                                         const std::string& label,
                                         const std::string& title) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep ambiguous stable selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep ambiguous stable selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1708: deep ambiguous stable section/settings selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1708: deep ambiguous stable section/settings selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1708: deep ambiguous stable label section/settings selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1708: deep ambiguous stable selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1708: deep ambiguous stable selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1708: deep ambiguous stable selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1708: deep ambiguous stable selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1708: deep ambiguous stable selectors should serialize null selected settings");
        expect_contains(process.stdout_text, "\"liveObjectCount\": 8",
                        "#1708: deep ambiguous stable selectors should parse objects beyond the default preview record limit");
    };

    const auto run_deep_ambiguous_section = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_deep_ambiguous_section_fixture(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-section-guid", "--json"},
            temp_root);

        expect_no_selection(section_process, label, title);
        expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                        "#1708: deep ambiguous section selectors should preserve both live sections");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1708: deep ambiguous section selectors should preserve deleted section counts");
        expect_not_contains(section_process.stdout_text, "\"selectedReportSection\": {",
                            "#1708: deep ambiguous section selectors should not select the preview-window section");
    };

    const auto run_deep_ambiguous_settings = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_deep_ambiguous_settings_fixture(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-settings-guid", "--json"},
            temp_root);

        expect_no_selection(settings_process, label, title);
        expect_contains(settings_process.stdout_text, "\"sectionCount\": 1",
                        "#1708: deep ambiguous settings selectors should preserve live section counts");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1708: deep ambiguous settings selectors should preserve both root settings rows");
        expect_not_contains(settings_process.stdout_text, "\"selectedReportSettings\": [",
                            "#1708: deep ambiguous settings selectors should not select the preview-window settings row");
    };

    run_deep_ambiguous_section(temp_root / "deep_ambiguous_section.frx",
                               "deep_ambiguous_section.frx",
                               "report section");
    run_deep_ambiguous_section(temp_root / "deep_ambiguous_section.lbx",
                               "deep_ambiguous_section.lbx",
                               "label section");
    run_deep_ambiguous_settings(temp_root / "deep_ambiguous_settings.frx",
                                "deep_ambiguous_settings.frx",
                                "report settings");
    run_deep_ambiguous_settings(temp_root / "deep_ambiguous_settings.lbx",
                                "deep_ambiguous_settings.lbx",
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

    cf_test_studio_host_json::test_studio_host_json_selects_deep_report_sections_and_settings_by_stable_selector(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_report_selection_for_deep_ambiguous_section_and_settings_selectors(argv[1]);
    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
