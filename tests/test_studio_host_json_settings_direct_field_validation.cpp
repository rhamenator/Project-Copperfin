// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_blank_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATIO", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "", "   ", "\t", " \t ", "", "  ", "\t ", "", "   ",
         "blank-direct-live-settings-guid"},
        {"1", "53", " ", "", "  ", "\t\t", " \t", "", " ", "\t", "",
         "blank-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1743: synthetic report table with blank direct-setting fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1743: synthetic report table should mark blank direct settings deleted");
}

void write_synthetic_report_table_for_mixed_invalid_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATIO", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "1", "paper?", "120", "bottom?", "1", "wide-grid?", "3", "wide?", "42",
         "mixed-invalid-live-settings-guid"},
        {"1", "53", "deleted-sideways", "9", "deleted-top?", "240", "deleted-grid?", "0",
         "deleted-many?", "5000", "deleted-spacing?", "mixed-invalid-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1744: synthetic report table with mixed invalid direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1744: synthetic report table should mark mixed invalid direct settings deleted");
}

void write_synthetic_report_table_for_trimmed_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATIO", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", " 1 ", "\t9", " 120\t", "\t240 ", " 1 ", "\t0\t", " 3 ", " 5000 ",
         "\t42", "trimmed-direct-live-settings-guid"},
        {"1", "53", "\t2", " 10 ", "\t360", "480\t", " 0", " 1 ", "\t4\t", " 6000",
         "84 ", "trimmed-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1745: synthetic report table with trimmed direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1745: synthetic report table should mark trimmed direct settings deleted");
}

void write_synthetic_report_table_for_fractional_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATIO", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "1.9", "9.8", "120.75", "240.25", "1.1", "0.9", "3.5", "5000.99",
         "42.42", "fractional-direct-live-settings-guid"},
        {"1", "53", "2.1", "10.9", "360.5", "480.5", "0.1", "1.1", "4.9", "6000.5",
         "84.9", "fractional-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1746: synthetic report table with fractional direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1746: synthetic report table should mark fractional direct settings deleted");
}

void write_synthetic_report_table_for_oversized_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATIO", .type = 'C', .length = 32U},
        {.name = "PAPERSIZE", .type = 'C', .length = 32U},
        {.name = "TOPMARGIN", .type = 'C', .length = 32U},
        {.name = "BOTMARGIN", .type = 'C', .length = 32U},
        {.name = "GRIDV", .type = 'C', .length = 32U},
        {.name = "GRIDH", .type = 'C', .length = 32U},
        {.name = "COLS", .type = 'C', .length = 32U},
        {.name = "COLWIDTH", .type = 'C', .length = 32U},
        {.name = "COLSPACING", .type = 'C', .length = 32U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "999999999999", "999999999998", "999999999997", "999999999996",
         "999999999995", "999999999994", "999999999993", "999999999992", "999999999991",
         "oversized-direct-live-settings-guid"},
        {"1", "53", "888888888888", "888888888887", "888888888886", "888888888885",
         "888888888884", "888888888883", "888888888882", "888888888881", "888888888880",
         "oversized-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1747: synthetic report table with oversized direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1747: synthetic report table should mark oversized direct settings deleted");
}

void write_synthetic_report_table_for_dot_leading_direct_setting_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATIO", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "BOTMARGIN", .type = 'C', .length = 24U},
        {.name = "GRIDV", .type = 'C', .length = 24U},
        {.name = "GRIDH", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "COLWIDTH", .type = 'C', .length = 24U},
        {.name = "COLSPACING", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", ".1", ".9", ".120", ".240", ".1", ".0", ".3", ".5000", ".42",
         "dot-leading-direct-live-settings-guid"},
        {"1", "53", ".2", ".10", ".360", ".480", ".0", ".1", ".4", ".6000", ".84",
         "dot-leading-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1748: synthetic report table with dot-leading direct settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1748: synthetic report table should mark dot-leading direct settings deleted");
}

#if !defined(COPPERFIN_REPORT_DIRECT_SETTING_FIELDS_SKIP_HOST_SMOKE)
void test_studio_host_json_skips_blank_report_direct_setting_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_blank_direct_setting_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_blank_direct_setting_layout = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_blank_direct_setting_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " blank direct setting summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " blank direct setting summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1743: blank direct-setting fields should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1743: blank direct-setting layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1743: blank direct-setting label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 0",
                        "#1743: blank direct-setting fields should not become live settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1743: blank direct-setting fields should not become deleted settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1743: blank direct-setting fields should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1743: blank direct-setting fields should not fabricate column setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1743: blank orientation fields should not advertise orientation availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1743: blank paper-size fields should not advertise paper-size availability");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": false",
                        "#1743: blank top-margin fields should not advertise top-margin availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1743: blank bottom-margin fields should not advertise bottom-margin availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1743: blank vertical-grid fields should not advertise vertical-grid availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1743: blank horizontal-grid fields should not advertise horizontal-grid availability");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1743: blank column-count fields should not advertise column-count availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1743: blank column-width fields should not advertise column-width availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1743: blank column-spacing fields should not advertise column-spacing availability");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1743: blank live direct-setting selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1743: blank live direct-setting fields should not expose selected settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1743: blank live direct-setting selected settings should be null");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1743: blank deleted direct-setting selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1743: blank deleted direct-setting fields should not expose selected settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1743: blank deleted direct-setting selected settings should be null");
    };

    run_blank_direct_setting_layout(temp_root / "blank_direct_setting.frx",
                                    "blank_direct_setting.frx",
                                    "report");
    run_blank_direct_setting_layout(temp_root / "blank_direct_setting.lbx",
                                    "blank_direct_setting.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_mixed_invalid_report_direct_setting_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_mixed_invalid_direct_setting_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_mixed_invalid_direct_setting_layout = [&](const fs::path& asset_path,
                                                             const std::string& title,
                                                             const std::string& label) {
        write_synthetic_report_table_for_mixed_invalid_direct_setting_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " mixed invalid direct setting summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " mixed invalid direct setting summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1744: mixed invalid direct settings should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1744: mixed invalid direct-setting layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1744: mixed invalid direct-setting label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 9",
                        "#1744: mixed invalid direct settings should preserve live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 9",
                        "#1744: mixed invalid direct settings should preserve deleted raw settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1744: mixed invalid direct settings should keep valid live page setup available");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": true",
                        "#1744: mixed invalid direct settings should keep valid live orientation available");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1744: mixed invalid direct settings should keep valid live orientation code");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1744: mixed invalid direct settings should not parse invalid live paper size");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": true",
                        "#1744: mixed invalid direct settings should keep valid live top margin available");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1744: mixed invalid direct settings should keep valid live top margin");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1744: mixed invalid direct settings should not parse invalid live bottom margin");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": true",
                        "#1744: mixed invalid direct settings should keep valid live vertical grid available");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 1",
                        "#1744: mixed invalid direct settings should keep valid live vertical grid");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1744: mixed invalid direct settings should not parse invalid live horizontal grid");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1744: mixed invalid direct settings should keep valid live column setup available");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": true",
                        "#1744: mixed invalid direct settings should keep valid live column count available");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1744: mixed invalid direct settings should keep valid live column count");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1744: mixed invalid direct settings should not parse invalid live column width");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": true",
                        "#1744: mixed invalid direct settings should keep valid live column spacing available");
        expect_contains(summary_process.stdout_text, "\"columnSpacing\": 42",
                        "#1744: mixed invalid direct settings should keep valid live column spacing");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2319: mixed invalid direct settings summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1744: mixed invalid live direct-setting selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1744: mixed invalid live direct-setting selection should expose raw settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1744: mixed invalid live direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2319: selected mixed invalid live direct settings JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 3",
                "\"value\": \"paper?\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"120\"",
                "\"name\": \"BOTMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 5",
                "\"value\": \"bottom?\"",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 6",
                "\"value\": \"1\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 7",
                "\"value\": \"wide-grid?\"",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 8",
                "\"value\": \"3\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 9",
                "\"value\": \"wide?\"",
                "\"name\": \"COLSPACING\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 10",
                "\"value\": \"42\""
            },
            "#1744: mixed invalid live direct-setting selection should expose valid and invalid raw settings");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1744: mixed invalid deleted direct-setting selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1744: mixed invalid deleted direct-setting selection should expose raw settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1744: mixed invalid deleted direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2319: selected mixed invalid deleted direct settings JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"value\": \"deleted-sideways\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"9\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 4",
                "\"value\": \"deleted-top?\"",
                "\"name\": \"BOTMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 5",
                "\"value\": \"240\"",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 6",
                "\"value\": \"deleted-grid?\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 7",
                "\"value\": \"0\"",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 8",
                "\"value\": \"deleted-many?\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 9",
                "\"value\": \"5000\"",
                "\"name\": \"COLSPACING\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 10",
                "\"value\": \"deleted-spacing?\""
            },
            "#1744: mixed invalid deleted direct-setting selection should expose valid and invalid raw settings");
    };

    run_mixed_invalid_direct_setting_layout(temp_root / "mixed_invalid_direct_setting.frx",
                                            "mixed_invalid_direct_setting.frx",
                                            "report");
    run_mixed_invalid_direct_setting_layout(temp_root / "mixed_invalid_direct_setting.lbx",
                                            "mixed_invalid_direct_setting.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_trimmed_report_direct_setting_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_trimmed_direct_setting_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_trimmed_direct_setting_layout = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_trimmed_direct_setting_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " trimmed direct setting summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " trimmed direct setting summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1745: trimmed direct settings should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1745: trimmed direct-setting layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1745: trimmed direct-setting label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 9",
                        "#1745: trimmed direct settings should preserve live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 9",
                        "#1745: trimmed direct settings should preserve deleted raw settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1745: trimmed direct settings should expose page setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": true",
                        "#1745: trimmed direct settings should expose orientation");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1745: trimmed direct settings should parse orientation");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": true",
                        "#1745: trimmed direct settings should expose paper size");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1745: trimmed direct settings should parse paper size");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": true",
                        "#1745: trimmed direct settings should expose top margin");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1745: trimmed direct settings should parse top margin");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": true",
                        "#1745: trimmed direct settings should expose bottom margin");
        expect_contains(summary_process.stdout_text, "\"bottomMargin\": 240",
                        "#1745: trimmed direct settings should parse bottom margin");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": true",
                        "#1745: trimmed direct settings should expose vertical grid");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 1",
                        "#1745: trimmed direct settings should parse vertical grid");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": true",
                        "#1745: trimmed direct settings should expose horizontal grid");
        expect_contains(summary_process.stdout_text, "\"gridHorizontal\": 0",
                        "#1745: trimmed direct settings should parse horizontal grid");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1745: trimmed direct settings should expose column setup");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": true",
                        "#1745: trimmed direct settings should expose column count");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1745: trimmed direct settings should parse column count");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": true",
                        "#1745: trimmed direct settings should expose column width");
        expect_contains(summary_process.stdout_text, "\"columnWidth\": 5000",
                        "#1745: trimmed direct settings should parse column width");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": true",
                        "#1745: trimmed direct settings should expose column spacing");
        expect_contains(summary_process.stdout_text, "\"columnSpacing\": 42",
                        "#1745: trimmed direct settings should parse column spacing");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2320: trimmed direct settings summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1745: trimmed live direct-setting selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1745: trimmed live direct-setting selection should expose raw settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1745: trimmed live direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2320: selected trimmed live direct settings JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"value\": \"9\"",
                "\"name\": \"TOPMARGIN\"",
                "\"value\": \"120\"",
                "\"name\": \"BOTMARGIN\"",
                "\"value\": \"240\"",
                "\"name\": \"GRIDV\"",
                "\"value\": \"1\"",
                "\"name\": \"GRIDH\"",
                "\"value\": \"0\"",
                "\"name\": \"COLS\"",
                "\"value\": \"3\"",
                "\"name\": \"COLWIDTH\"",
                "\"value\": \"5000\"",
                "\"name\": \"COLSPACING\"",
                "\"value\": \"42\""
            },
            "#1745: trimmed live direct-setting selection should expose trimmed raw settings");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1745: trimmed deleted direct-setting selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1745: trimmed deleted direct-setting selection should expose raw settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1745: trimmed deleted direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2320: selected trimmed deleted direct settings JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"value\": \"2\"",
                "\"name\": \"PAPERSIZE\"",
                "\"value\": \"10\"",
                "\"name\": \"TOPMARGIN\"",
                "\"value\": \"360\"",
                "\"name\": \"BOTMARGIN\"",
                "\"value\": \"480\"",
                "\"name\": \"GRIDV\"",
                "\"value\": \"0\"",
                "\"name\": \"GRIDH\"",
                "\"value\": \"1\"",
                "\"name\": \"COLS\"",
                "\"value\": \"4\"",
                "\"name\": \"COLWIDTH\"",
                "\"value\": \"6000\"",
                "\"name\": \"COLSPACING\"",
                "\"value\": \"84\""
            },
            "#1745: trimmed deleted direct-setting selection should expose trimmed raw settings");
    };

    run_trimmed_direct_setting_layout(temp_root / "trimmed_direct_setting.frx",
                                      "trimmed_direct_setting.frx",
                                      "report");
    run_trimmed_direct_setting_layout(temp_root / "trimmed_direct_setting.lbx",
                                      "trimmed_direct_setting.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_fractional_report_direct_setting_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_fractional_direct_setting_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_fractional_direct_setting_layout = [&](const fs::path& asset_path,
                                                          const std::string& title,
                                                          const std::string& label) {
        write_synthetic_report_table_for_fractional_direct_setting_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " fractional direct setting summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " fractional direct setting summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1746: fractional direct settings should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1746: fractional direct-setting layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1746: fractional direct-setting label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 9",
                        "#1746: fractional direct settings should preserve live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 9",
                        "#1746: fractional direct settings should preserve deleted raw settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1746: fractional direct settings should expose page setup");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1746: fractional direct settings should parse orientation integer portion");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1746: fractional direct settings should parse paper-size integer portion");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1746: fractional direct settings should parse top-margin integer portion");
        expect_contains(summary_process.stdout_text, "\"bottomMargin\": 240",
                        "#1746: fractional direct settings should parse bottom-margin integer portion");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 1",
                        "#1746: fractional direct settings should parse vertical-grid integer portion");
        expect_contains(summary_process.stdout_text, "\"gridHorizontal\": 0",
                        "#1746: fractional direct settings should parse horizontal-grid integer portion");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1746: fractional direct settings should expose column setup");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1746: fractional direct settings should parse column-count integer portion");
        expect_contains(summary_process.stdout_text, "\"columnWidth\": 5000",
                        "#1746: fractional direct settings should parse column-width integer portion");
        expect_contains(summary_process.stdout_text, "\"columnSpacing\": 42",
                        "#1746: fractional direct settings should parse column-spacing integer portion");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2321: fractional direct settings summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1746: fractional live direct-setting selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1746: fractional live direct-setting selection should expose raw settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1746: fractional live direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2321: selected fractional live direct settings JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"value\": \"1.9\"",
                "\"name\": \"PAPERSIZE\"",
                "\"value\": \"9.8\"",
                "\"name\": \"TOPMARGIN\"",
                "\"value\": \"120.75\"",
                "\"name\": \"BOTMARGIN\"",
                "\"value\": \"240.25\"",
                "\"name\": \"GRIDV\"",
                "\"value\": \"1.1\"",
                "\"name\": \"GRIDH\"",
                "\"value\": \"0.9\"",
                "\"name\": \"COLS\"",
                "\"value\": \"3.5\"",
                "\"name\": \"COLWIDTH\"",
                "\"value\": \"5000.99\"",
                "\"name\": \"COLSPACING\"",
                "\"value\": \"42.42\""
            },
            "#1746: fractional live direct-setting selection should expose fractional source values");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1746: fractional deleted direct-setting selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1746: fractional deleted direct-setting selection should expose raw settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1746: fractional deleted direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2321: selected fractional deleted direct settings JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"value\": \"2.1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"value\": \"10.9\"",
                "\"name\": \"TOPMARGIN\"",
                "\"value\": \"360.5\"",
                "\"name\": \"BOTMARGIN\"",
                "\"value\": \"480.5\"",
                "\"name\": \"GRIDV\"",
                "\"value\": \"0.1\"",
                "\"name\": \"GRIDH\"",
                "\"value\": \"1.1\"",
                "\"name\": \"COLS\"",
                "\"value\": \"4.9\"",
                "\"name\": \"COLWIDTH\"",
                "\"value\": \"6000.5\"",
                "\"name\": \"COLSPACING\"",
                "\"value\": \"84.9\""
            },
            "#1746: fractional deleted direct-setting selection should expose fractional source values");
    };

    run_fractional_direct_setting_layout(temp_root / "fractional_direct_setting.frx",
                                         "fractional_direct_setting.frx",
                                         "report");
    run_fractional_direct_setting_layout(temp_root / "fractional_direct_setting.lbx",
                                         "fractional_direct_setting.lbx",
                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_oversized_report_direct_setting_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_oversized_direct_setting_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_oversized_direct_setting_layout = [&](const fs::path& asset_path,
                                                         const std::string& title,
                                                         const std::string& label) {
        write_synthetic_report_table_for_oversized_direct_setting_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " oversized direct setting summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " oversized direct setting summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1747: oversized direct settings should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1747: oversized direct-setting layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1747: oversized direct-setting label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 9",
                        "#1747: oversized direct settings should preserve live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 9",
                        "#1747: oversized direct settings should preserve deleted raw settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1747: oversized direct settings should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1747: oversized orientation should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1747: oversized paper size should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": false",
                        "#1747: oversized top margin should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1747: oversized bottom margin should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1747: oversized vertical grid should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1747: oversized horizontal grid should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1747: oversized direct settings should not fabricate column setup");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1747: oversized column count should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1747: oversized column width should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1747: oversized column spacing should not advertise availability");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2322: oversized direct settings summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1747: oversized live direct-setting selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1747: oversized live direct-setting selection should expose raw settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1747: oversized live direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2322: selected oversized live direct settings JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"value\": \"999999999999\"",
                "\"name\": \"PAPERSIZE\"",
                "\"value\": \"999999999998\"",
                "\"name\": \"TOPMARGIN\"",
                "\"value\": \"999999999997\"",
                "\"name\": \"BOTMARGIN\"",
                "\"value\": \"999999999996\"",
                "\"name\": \"GRIDV\"",
                "\"value\": \"999999999995\"",
                "\"name\": \"GRIDH\"",
                "\"value\": \"999999999994\"",
                "\"name\": \"COLS\"",
                "\"value\": \"999999999993\"",
                "\"name\": \"COLWIDTH\"",
                "\"value\": \"999999999992\"",
                "\"name\": \"COLSPACING\"",
                "\"value\": \"999999999991\""
            },
            "#1747: oversized live direct-setting selection should expose oversized source values");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1747: oversized deleted direct-setting selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1747: oversized deleted direct-setting selection should expose raw settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1747: oversized deleted direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2322: selected oversized deleted direct settings JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"value\": \"888888888888\"",
                "\"name\": \"PAPERSIZE\"",
                "\"value\": \"888888888887\"",
                "\"name\": \"TOPMARGIN\"",
                "\"value\": \"888888888886\"",
                "\"name\": \"BOTMARGIN\"",
                "\"value\": \"888888888885\"",
                "\"name\": \"GRIDV\"",
                "\"value\": \"888888888884\"",
                "\"name\": \"GRIDH\"",
                "\"value\": \"888888888883\"",
                "\"name\": \"COLS\"",
                "\"value\": \"888888888882\"",
                "\"name\": \"COLWIDTH\"",
                "\"value\": \"888888888881\"",
                "\"name\": \"COLSPACING\"",
                "\"value\": \"888888888880\""
            },
            "#1747: oversized deleted direct-setting selection should expose oversized source values");
    };

    run_oversized_direct_setting_layout(temp_root / "oversized_direct_setting.frx",
                                        "oversized_direct_setting.frx",
                                        "report");
    run_oversized_direct_setting_layout(temp_root / "oversized_direct_setting.lbx",
                                        "oversized_direct_setting.lbx",
                                        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_dot_leading_report_direct_setting_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_dot_leading_direct_setting_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_dot_leading_direct_setting_layout = [&](const fs::path& asset_path,
                                                           const std::string& title,
                                                           const std::string& label) {
        write_synthetic_report_table_for_dot_leading_direct_setting_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " dot-leading direct setting summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " dot-leading direct setting summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1748: dot-leading direct settings should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1748: dot-leading direct-setting layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1748: dot-leading direct-setting label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 9",
                        "#1748: dot-leading direct settings should preserve live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 9",
                        "#1748: dot-leading direct settings should preserve deleted raw settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1748: dot-leading direct settings should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1748: dot-leading orientation should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1748: dot-leading paper size should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": false",
                        "#1748: dot-leading top margin should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1748: dot-leading bottom margin should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1748: dot-leading vertical grid should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1748: dot-leading horizontal grid should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1748: dot-leading direct settings should not fabricate column setup");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1748: dot-leading column count should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1748: dot-leading column width should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1748: dot-leading column spacing should not advertise availability");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2323: dot-leading direct settings summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1748: dot-leading live direct-setting selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1748: dot-leading live direct-setting selection should expose raw settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1748: dot-leading live direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2323: selected dot-leading live direct settings JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"value\": \".1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"value\": \".9\"",
                "\"name\": \"TOPMARGIN\"",
                "\"value\": \".120\"",
                "\"name\": \"BOTMARGIN\"",
                "\"value\": \".240\"",
                "\"name\": \"GRIDV\"",
                "\"value\": \".1\"",
                "\"name\": \"GRIDH\"",
                "\"value\": \".0\"",
                "\"name\": \"COLS\"",
                "\"value\": \".3\"",
                "\"name\": \"COLWIDTH\"",
                "\"value\": \".5000\"",
                "\"name\": \"COLSPACING\"",
                "\"value\": \".42\""
            },
            "#1748: dot-leading live direct-setting selection should expose source values");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1748: dot-leading deleted direct-setting selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1748: dot-leading deleted direct-setting selection should expose raw settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1748: dot-leading deleted direct-setting selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2323: selected dot-leading deleted direct settings JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"value\": \".2\"",
                "\"name\": \"PAPERSIZE\"",
                "\"value\": \".10\"",
                "\"name\": \"TOPMARGIN\"",
                "\"value\": \".360\"",
                "\"name\": \"BOTMARGIN\"",
                "\"value\": \".480\"",
                "\"name\": \"GRIDV\"",
                "\"value\": \".0\"",
                "\"name\": \"GRIDH\"",
                "\"value\": \".1\"",
                "\"name\": \"COLS\"",
                "\"value\": \".4\"",
                "\"name\": \"COLWIDTH\"",
                "\"value\": \".6000\"",
                "\"name\": \"COLSPACING\"",
                "\"value\": \".84\""
            },
            "#1748: dot-leading deleted direct-setting selection should expose source values");
    };

    run_dot_leading_direct_setting_layout(temp_root / "dot_leading_direct_setting.frx",
                                          "dot_leading_direct_setting.frx",
                                          "report");
    run_dot_leading_direct_setting_layout(temp_root / "dot_leading_direct_setting.lbx",
                                          "dot_leading_direct_setting.lbx",
                                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

}  // namespace cf_test_studio_host_json
