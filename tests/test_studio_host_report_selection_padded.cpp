// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
namespace {

void write_padded_stable_object_fixture(const std::filesystem::path& report_path) {
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
        {"5", "", "\"Padded label\"", "400", "1200", "1500", "250", "  padded-object-guid  "}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1703: synthetic report table for padded stable object JSON should be created");
}

void write_padded_stable_section_fixture(const std::filesystem::path& report_path) {
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
        {"9", "4", "", "0", "3200", "  padded-section-guid  "}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1703: synthetic report table for padded stable section JSON should be created");
}

void write_padded_stable_settings_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "  padded-settings-guid  "},
        {"9", "4", "", "0", "3200", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1703: synthetic report table for padded stable settings JSON should be created");
}

}  // namespace

void test_studio_host_json_selects_padded_report_records_by_trimmed_stable_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_padded_report_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_common_selection = [&](const ProcessResult& process,
                                             const std::string& label,
                                             const std::string& title,
                                             const std::string& selection_kind) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " padded stable selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " padded stable selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1703: padded stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1703: padded stable selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1703: padded stable label selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1703: padded stable selectors should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"" + selection_kind + "\"",
                        "#1703: padded stable selectors should expose the selected category");
    };

    const auto run_padded_object_selector = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_padded_stable_object_fixture(asset_path);

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", " PADDED-OBJECT-GUID ", "--json"},
            temp_root);

        expect_common_selection(object_process, label, title, "object");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1703: padded object selectors should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1703: padded object selectors should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1703: padded object selectors should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1703: padded object selectors should expose containing-section availability");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"title\": \"\\\"Padded label\\\"\"",
                "\"objectKind\": \"label\"",
                "\"recordIndex\": 2",
                "\"deleted\": false"
            },
            "#1703: padded object selectors should expose the selected report object");
    };

    const auto run_padded_section_selector = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_padded_stable_section_fixture(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", " padded-section-guid ", "--json"},
            temp_root);

        expect_common_selection(section_process, label, title, "section");
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1703: padded section selectors should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1703: padded section selectors should not advertise selected-object availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1703: padded section selectors should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1703: padded section selectors should not advertise containing-section availability");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1"
            },
            "#1703: padded section selectors should expose the selected report section");
    };

    const auto run_padded_settings_selector = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_padded_stable_settings_fixture(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", " Padded-Settings-Guid ", "--json"},
            temp_root);

        expect_common_selection(settings_process, label, title, "settings");
        expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1703: padded settings selectors should advertise selected-settings availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1703: padded settings selectors should not advertise selected-section availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1703: padded settings selectors should not advertise selected-object availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1703: padded settings selectors should not advertise containing-section availability");
        expect_contains(settings_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1703: padded settings selectors should preserve live page setup summaries");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"value\": \"0\""
            },
            "#1703: padded settings selectors should expose the selected root settings");
    };

    run_padded_object_selector(temp_root / "padded_object.frx",
                               "padded_object.frx",
                               "report object");
    run_padded_object_selector(temp_root / "padded_object.lbx",
                               "padded_object.lbx",
                               "label object");
    run_padded_section_selector(temp_root / "padded_section.frx",
                                "padded_section.frx",
                                "report section");
    run_padded_section_selector(temp_root / "padded_section.lbx",
                                "padded_section.lbx",
                                "label section");
    run_padded_settings_selector(temp_root / "padded_settings.frx",
                                 "padded_settings.frx",
                                 "report settings");
    run_padded_settings_selector(temp_root / "padded_settings.lbx",
                                 "padded_settings.lbx",
                                 "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_record_selection_takes_precedence_over_stable_report_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_record_selector_precedence_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_common_record_selection = [&](const ProcessResult& process,
                                                    const std::string& label,
                                                    const std::string& title,
                                                    const std::string& selection_kind) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " record selector precedence stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " record selector precedence stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1704: report/label record selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1704: record selector precedence should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1704: record selector precedence should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1704: record selector precedence should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"" + selection_kind + "\"",
                        "#1704: record selector precedence should expose the record-selected category");
    };

    const auto run_section_record_precedence = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_padded_stable_object_fixture(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "1",
                "--unique-id", "padded-object-guid",
                "--json"
            },
            temp_root);

        expect_common_record_selection(section_process, label, title, "section");
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1704: section record selectors should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1704: section record selectors should not switch to conflicting selected objects");
        expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                        "#1704: section record selectors should serialize null selected objects");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1704: section record selectors should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1704: section record selectors should serialize null selected settings");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"objectCount\": 1"
            },
            "#1704: section record selectors should preserve selected-section metadata despite conflicting unique-id");
    };

    const auto run_settings_record_precedence = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_padded_stable_object_fixture(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "0",
                "--unique-id", "padded-object-guid",
                "--json"
            },
            temp_root);

        expect_common_record_selection(settings_process, label, title, "settings");
        expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1704: settings record selectors should advertise selected-settings availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1704: settings record selectors should not switch to conflicting selected objects");
        expect_contains(settings_process.stdout_text, "\"selectedReportObject\": null",
                        "#1704: settings record selectors should serialize null selected objects");
        expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1704: settings record selectors should not advertise selected-section availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportSection\": null",
                        "#1704: settings record selectors should serialize null selected sections");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"value\": \"0\""
            },
            "#1704: settings record selectors should preserve selected-settings metadata despite conflicting unique-id");
    };

    run_section_record_precedence(temp_root / "record_section_precedence.frx",
                                  "record_section_precedence.frx",
                                  "report section");
    run_section_record_precedence(temp_root / "record_section_precedence.lbx",
                                  "record_section_precedence.lbx",
                                  "label section");
    run_settings_record_precedence(temp_root / "record_settings_precedence.frx",
                                   "record_settings_precedence.frx",
                                   "report settings");
    run_settings_record_precedence(temp_root / "record_settings_precedence.lbx",
                                   "record_settings_precedence.lbx",
                                   "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_out_of_range_report_record_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_out_of_range_report_record_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_out_of_range_record_selection = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_padded_stable_object_fixture(asset_path);

        const auto process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "99",
                "--unique-id", "padded-object-guid",
                "--json"
            },
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " out-of-range record selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " out-of-range record selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1713: out-of-range report/label record selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1713: out-of-range record selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1713: out-of-range record selectors should retain label identity");
        }
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"launchSelection\": {",
                "\"recordAvailable\": false",
                "\"recordIndex\": 99"
            },
            "#1713: out-of-range record selectors should preserve the requested record index without marking it available");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1713: out-of-range record selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1713: out-of-range record selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1713: out-of-range record selectors should not fall back to conflicting unique-id objects");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1713: out-of-range record selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1713: out-of-range record selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1713: out-of-range record selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1713: out-of-range record selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1713: out-of-range record selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1713: out-of-range record selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1713: out-of-range record selectors should serialize null selected settings");
        expect_contains(process.stdout_text, "\"sectionCount\": 1",
                        "#1713: out-of-range record selectors should preserve live section counts");
        expect_contains(process.stdout_text, "\"settingCount\": 1",
                        "#1713: out-of-range record selectors should preserve live setting counts");
        expect_contains(process.stdout_text, "\"liveObjectCount\": 1",
                        "#1713: out-of-range record selectors should preserve live object counts");
        expect_not_contains(process.stdout_text, "\"selectedReportObject\": {",
                            "#1713: out-of-range record selectors should not fall back to the valid unique-id selector");
    };

    run_out_of_range_record_selection(temp_root / "out_of_range_record.frx",
                                      "out_of_range_record.frx",
                                      "report");
    run_out_of_range_record_selection(temp_root / "out_of_range_record.lbx",
                                      "out_of_range_record.lbx",
                                      "label");

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

    cf_test_studio_host_json::test_studio_host_json_selects_padded_report_records_by_trimmed_stable_selector(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_record_selection_takes_precedence_over_stable_report_selector(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_out_of_range_report_record_selectors(argv[1]);
    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
