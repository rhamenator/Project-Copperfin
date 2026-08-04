// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_unresolved_direct_setting_memo_layout_json(
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
        {"1", "53", "<memo block 80>", "<memo block 81>", "<memo block 82>", "<memo block 83>",
         "<memo block 84>", "<memo block 85>", "<memo block 86>", "<memo block 87>",
         "<memo block 88>", "unresolved-direct-live-settings-guid"},
        {"1", "53", "<memo block 89>", "<memo block 90>", "<memo block 91>", "<memo block 92>",
         "<memo block 93>", "<memo block 94>", "<memo block 95>", "<memo block 96>",
         "<memo block 97>", "unresolved-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1741: synthetic report table with unresolved direct-setting memo placeholders should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok,
           "#1741: synthetic report table should mark unresolved direct-setting memo settings deleted");
}

void write_synthetic_report_table_for_mixed_direct_setting_memo_layout_json(
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
        {"1", "53", "1", "<memo block 100>", "120", "<memo block 101>", "1",
         "<memo block 102>", "3", "<memo block 103>", "42", "mixed-direct-live-settings-guid"},
        {"1", "53", "<memo block 104>", "9", "<memo block 105>", "240", "<memo block 106>",
         "0", "<memo block 107>", "5000", "<memo block 108>", "mixed-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1742: synthetic report table with mixed direct-setting memo placeholders should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok,
           "#1742: synthetic report table should mark mixed direct-setting memo settings deleted");
}

void write_synthetic_report_table_for_invalid_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "ORIENTATION=sideways\n"
         "PAPERSIZE=999999999999\n"
         "TOPMARGIN=.120\n"
         "BOTMARGIN=bottom?\n"
         "GRIDV=999999999995\n"
         "GRIDH=.0\n"
         "COLS=many\n"
         "COLWIDTH=999999999992\n"
         "COLSPACING=.42",
         "invalid-memo-live-settings-guid"},
        {"1", "53",
         "ORIENTATION=deleted-sideways\n"
         "PAPERSIZE=888888888887\n"
         "TOPMARGIN=.360\n"
         "BOTMARGIN=deleted-bottom?\n"
         "GRIDV=888888888884\n"
         "GRIDH=.1\n"
         "COLS=deleted-many\n"
         "COLWIDTH=888888888881\n"
         "COLSPACING=.84",
         "invalid-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1750: synthetic report table with invalid settings memo values should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1750: synthetic report table should mark invalid memo settings deleted");
}

void write_synthetic_report_table_for_fractional_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         " ORIENTATION = 1.9 \n"
         "PAPERSIZE=\t9.8\n"
         " TOPMARGIN = 120.75\n"
         "BOTMARGIN=240.25\n"
         "GRIDV=1.1\n"
         "GRIDH=0.9\n"
         "COLS=3.5\n"
         "COLWIDTH=5000.99\n"
         "COLSPACING=42.42",
         "fractional-memo-live-settings-guid"},
        {"1", "53",
         " ORIENTATION = 2.1\n"
         "PAPERSIZE= 10.9\n"
         "TOPMARGIN=360.5\n"
         "BOTMARGIN=480.5\n"
         "GRIDV=0.1\n"
         "GRIDH=1.1\n"
         "COLS=4.9\n"
         "COLWIDTH=6000.5\n"
         "COLSPACING=84.9",
         "fractional-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1751: synthetic report table with fractional settings memo values should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1751: synthetic report table should mark fractional memo settings deleted");
}

void write_synthetic_report_table_for_blank_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "ORIENTATION=\n"
         "PAPERSIZE=   \n"
         "TOPMARGIN=\t\n"
         "BOTMARGIN= \t \n"
         "GRIDV=\n"
         "GRIDH=  \n"
         "COLS=\t \n"
         "COLWIDTH=\n"
         "COLSPACING=   ",
         "blank-memo-live-settings-guid"},
        {"1", "53",
         "ORIENTATION= \n"
         "PAPERSIZE=\n"
         "TOPMARGIN=  \n"
         "BOTMARGIN=\t\t\n"
         "GRIDV= \t\n"
         "GRIDH=\n"
         "COLS= \n"
         "COLWIDTH=\t\n"
         "COLSPACING=",
         "blank-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1752: synthetic report table with blank settings memo values should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1752: synthetic report table should mark blank memo settings deleted");
}

void write_synthetic_report_table_for_malformed_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "orphan live memo text\n"
         " = ignored-live-name\n"
         "ORIENTATION=1\r\n"
         "live no equals\r\n"
         "PAPERSIZE=9\n"
         " = \n"
         "COLS=3",
         "malformed-memo-live-settings-guid"},
        {"1", "53",
         "deleted orphan text\n"
         " = ignored-deleted-name\r\n"
         "TOPMARGIN=120\n"
         "deleted no equals\n"
         "BOTMARGIN=240\r\n"
         " =\n"
         "COLWIDTH=5000\r\n"
         "COLSPACING=42",
         "malformed-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1753: synthetic report table with malformed settings memo lines should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1753: synthetic report table should mark malformed memo settings deleted");
}

void write_synthetic_report_table_for_cr_only_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "ORIENTATION=1\r"
         "PAPERSIZE=9\r"
         "COLS=3\r"
         "COLWIDTH=5000",
         "cr-only-memo-live-settings-guid"},
        {"1", "53",
         "TOPMARGIN=120\r"
         "BOTMARGIN=240\r"
         "COLSPACING=42",
         "cr-only-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1756: synthetic report table with CR-only settings memo lines should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1756: synthetic report table should mark CR-only memo settings deleted");
}

void write_synthetic_report_table_for_mixed_case_setting_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "orientation=1\n"
         "PaperSize=9\n"
         "TopMargin=120\n"
         "cols=3\n"
         "ColWidth=5000",
         "mixed-case-memo-live-settings-guid"},
        {"1", "53",
         "bottommargin=240\n"
         "GridV=1\n"
         "gridh=0\n"
         "ColSpacing=42",
         "mixed-case-memo-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1757: synthetic report table with mixed-case settings memo names should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1757: synthetic report table should mark mixed-case memo settings deleted");
}

#if !defined(COPPERFIN_REPORT_SETTINGS_MEMO_PARSING_SKIP_HOST_SMOKE)
void test_studio_host_json_suppresses_unresolved_report_direct_setting_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_direct_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_unresolved_direct_setting_memo_layout = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_unresolved_direct_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved direct setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved direct setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1741: unresolved direct-setting memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1741: unresolved direct-setting memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1741: unresolved direct-setting memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 0",
                        "#1741: unresolved direct-setting memo placeholders should not become live settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1741: unresolved direct-setting memo placeholders should not become deleted settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1741: unresolved direct-setting memo placeholders should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1741: unresolved direct-setting memo placeholders should not fabricate column setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1741: unresolved orientation placeholders should not advertise orientation availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1741: unresolved paper-size placeholders should not advertise paper-size availability");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": false",
                        "#1741: unresolved top-margin placeholders should not advertise top-margin availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1741: unresolved bottom-margin placeholders should not advertise bottom-margin availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1741: unresolved vertical-grid placeholders should not advertise vertical-grid availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1741: unresolved horizontal-grid placeholders should not advertise horizontal-grid availability");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1741: unresolved column-count placeholders should not advertise column-count availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1741: unresolved column-width placeholders should not advertise column-width availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1741: unresolved column-spacing placeholders should not advertise column-spacing availability");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1741: unresolved direct-setting memo placeholders should not leak into summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1741: unresolved live direct-setting memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1741: unresolved live direct-setting memo placeholders should not expose selected settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1741: unresolved live direct-setting memo selected settings should be null");
        expect_not_contains(live_settings_process.stdout_text, "<memo block",
                            "#1741: unresolved live direct-setting memo placeholders should not leak into selection JSON");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1741: unresolved deleted direct-setting memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1741: unresolved deleted direct-setting memo placeholders should not expose selected settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1741: unresolved deleted direct-setting memo selected settings should be null");
        expect_not_contains(deleted_settings_process.stdout_text, "<memo block",
                            "#1741: unresolved deleted direct-setting memo placeholders should not leak into selection JSON");
    };

    run_unresolved_direct_setting_memo_layout(temp_root / "unresolved_direct_setting_memo.frx",
                                              "unresolved_direct_setting_memo.frx",
                                              "report");
    run_unresolved_direct_setting_memo_layout(temp_root / "unresolved_direct_setting_memo.lbx",
                                              "unresolved_direct_setting_memo.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_mixed_report_direct_setting_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_mixed_direct_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_mixed_direct_setting_memo_layout = [&](const fs::path& asset_path,
                                                          const std::string& title,
                                                          const std::string& label) {
        write_synthetic_report_table_for_mixed_direct_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " mixed direct setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " mixed direct setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1742: mixed direct-setting memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1742: mixed direct-setting memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1742: mixed direct-setting memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 5",
                        "#1742: mixed direct-setting memo placeholders should preserve live valid settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#1742: mixed direct-setting memo placeholders should preserve deleted valid settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep live page setup available");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid orientation available");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1742: mixed direct-setting memo layouts should keep valid orientation code");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1742: mixed direct-setting memo layouts should suppress placeholder paper size");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid top margin available");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1742: mixed direct-setting memo layouts should keep valid top margin");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1742: mixed direct-setting memo layouts should suppress placeholder bottom margin");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid vertical grid available");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 1",
                        "#1742: mixed direct-setting memo layouts should keep valid vertical grid");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1742: mixed direct-setting memo layouts should suppress placeholder horizontal grid");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep live column setup available");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid column count available");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1742: mixed direct-setting memo layouts should keep valid column count");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1742: mixed direct-setting memo layouts should suppress placeholder column width");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": true",
                        "#1742: mixed direct-setting memo layouts should keep valid column spacing available");
        expect_contains(summary_process.stdout_text, "\"columnSpacing\": 42",
                        "#1742: mixed direct-setting memo layouts should keep valid column spacing");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1742: mixed direct-setting memo placeholders should not leak into summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1742: mixed live direct-setting memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1742: mixed live direct-setting memo selection should expose valid settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1742: mixed live direct-setting memo selection should expose settings kind");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"1\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"120\"",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 6",
                "\"value\": \"1\"",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 8",
                "\"value\": \"3\"",
                "\"name\": \"COLSPACING\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 10",
                "\"value\": \"42\""
            },
            "#1742: mixed live direct-setting memo selection should expose only valid settings");
        expect_not_contains(live_settings_process.stdout_text, "<memo block",
                            "#1742: mixed live direct-setting memo placeholders should not leak into selection JSON");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1742: mixed deleted direct-setting memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1742: mixed deleted direct-setting memo selection should expose valid settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1742: mixed deleted direct-setting memo selection should expose settings kind");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"9\"",
                "\"name\": \"BOTMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 5",
                "\"value\": \"240\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 7",
                "\"value\": \"0\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 9",
                "\"value\": \"5000\""
            },
            "#1742: mixed deleted direct-setting memo selection should expose only valid settings");
        expect_not_contains(deleted_settings_process.stdout_text, "<memo block",
                            "#1742: mixed deleted direct-setting memo placeholders should not leak into selection JSON");
    };

    run_mixed_direct_setting_memo_layout(temp_root / "mixed_direct_setting_memo.frx",
                                         "mixed_direct_setting_memo.frx",
                                         "report");
    run_mixed_direct_setting_memo_layout(temp_root / "mixed_direct_setting_memo.lbx",
                                         "mixed_direct_setting_memo.lbx",
                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_invalid_report_setting_memo_values(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_invalid_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_invalid_setting_memo_layout = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_invalid_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " invalid setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " invalid setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1750: invalid settings memo values should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1750: invalid settings memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1750: invalid settings memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 9",
                        "#1750: invalid settings memo values should preserve live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 9",
                        "#1750: invalid settings memo values should preserve deleted raw settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1750: invalid settings memo values should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1750: invalid memo orientation should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1750: invalid memo paper size should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": false",
                        "#1750: invalid memo top margin should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1750: invalid memo bottom margin should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1750: invalid memo vertical grid should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1750: invalid memo horizontal grid should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1750: invalid settings memo values should not fabricate column setup");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1750: invalid memo column count should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1750: invalid memo column width should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1750: invalid memo column spacing should not advertise availability");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"sideways\"",
                        "#1750: live malformed memo orientation provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"999999999999\"",
                        "#1750: live oversized memo paper-size provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2, \"memoBlockNumber\": 1, \"value\": \".120\"",
                        "#1750: live dot-leading memo top-margin provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLWIDTH\", \"recordIndex\": 1, \"fieldIndex\": 2, \"sourceLineIndex\": 7, \"memoBlockNumber\": 2, \"value\": \"888888888881\"",
                        "#1750: deleted oversized memo column-width provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLSPACING\", \"recordIndex\": 1, \"fieldIndex\": 2, \"sourceLineIndex\": 8, \"memoBlockNumber\": 2, \"value\": \".84\"",
                        "#1750: deleted dot-leading memo column-spacing provenance should remain inspectable");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2324: invalid settings memo summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1750: invalid live settings memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1750: invalid live settings memo selection should expose raw settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1750: invalid live settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2324: selected invalid live settings memo JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 1",
                "\"value\": \"sideways\"",
                "\"name\": \"PAPERSIZE\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"999999999999\"",
                "\"name\": \"TOPMARGIN\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \".120\"",
                "\"name\": \"BOTMARGIN\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"bottom?\"",
                "\"name\": \"GRIDV\"",
                "\"sourceLineIndex\": 4",
                "\"value\": \"999999999995\"",
                "\"name\": \"GRIDH\"",
                "\"sourceLineIndex\": 5",
                "\"value\": \".0\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 6",
                "\"value\": \"many\"",
                "\"name\": \"COLWIDTH\"",
                "\"sourceLineIndex\": 7",
                "\"value\": \"999999999992\"",
                "\"name\": \"COLSPACING\"",
                "\"sourceLineIndex\": 8",
                "\"value\": \".42\""
            },
            "#1750: invalid live settings memo selection should expose raw memo source values");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1750: invalid deleted settings memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1750: invalid deleted settings memo selection should expose raw settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1750: invalid deleted settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2324: selected invalid deleted settings memo JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 2",
                "\"value\": \"deleted-sideways\"",
                "\"name\": \"PAPERSIZE\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"888888888887\"",
                "\"name\": \"TOPMARGIN\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \".360\"",
                "\"name\": \"BOTMARGIN\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"deleted-bottom?\"",
                "\"name\": \"GRIDV\"",
                "\"sourceLineIndex\": 4",
                "\"value\": \"888888888884\"",
                "\"name\": \"GRIDH\"",
                "\"sourceLineIndex\": 5",
                "\"value\": \".1\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 6",
                "\"value\": \"deleted-many\"",
                "\"name\": \"COLWIDTH\"",
                "\"sourceLineIndex\": 7",
                "\"value\": \"888888888881\"",
                "\"name\": \"COLSPACING\"",
                "\"sourceLineIndex\": 8",
                "\"value\": \".84\""
            },
            "#1750: invalid deleted settings memo selection should expose raw memo source values");
    };

    run_invalid_setting_memo_layout(temp_root / "invalid_setting_memo.frx",
                                    "invalid_setting_memo.frx",
                                    "report");
    run_invalid_setting_memo_layout(temp_root / "invalid_setting_memo.lbx",
                                    "invalid_setting_memo.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_fractional_report_setting_memo_values(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_fractional_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_fractional_setting_memo_layout = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_fractional_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " fractional setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " fractional setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1751: fractional settings memo values should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1751: fractional settings memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1751: fractional settings memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 9",
                        "#1751: fractional settings memo values should preserve live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 9",
                        "#1751: fractional settings memo values should preserve deleted raw settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1751: fractional settings memo values should expose page setup");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1751: fractional memo orientation should parse integer portion");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1751: fractional memo paper size should parse integer portion");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1751: fractional memo top margin should parse integer portion");
        expect_contains(summary_process.stdout_text, "\"bottomMargin\": 240",
                        "#1751: fractional memo bottom margin should parse integer portion");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 1",
                        "#1751: fractional memo vertical grid should parse integer portion");
        expect_contains(summary_process.stdout_text, "\"gridHorizontal\": 0",
                        "#1751: fractional memo horizontal grid should parse integer portion");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1751: fractional settings memo values should expose column setup");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1751: fractional memo column count should parse integer portion");
        expect_contains(summary_process.stdout_text, "\"columnWidth\": 5000",
                        "#1751: fractional memo column width should parse integer portion");
        expect_contains(summary_process.stdout_text, "\"columnSpacing\": 42",
                        "#1751: fractional memo column spacing should parse integer portion");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"1.9\"",
                        "#1751: live trimmed memo orientation provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"9.8\"",
                        "#1751: live trimmed memo paper-size provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLSPACING\", \"recordIndex\": 1, \"fieldIndex\": 2, \"sourceLineIndex\": 8, \"memoBlockNumber\": 2, \"value\": \"84.9\"",
                        "#1751: deleted fractional memo column-spacing provenance should remain inspectable");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2325: fractional settings memo summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1751: fractional live settings memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1751: fractional live settings memo selection should expose raw settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1751: fractional live settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2325: selected fractional live settings memo JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 1",
                "\"value\": \"1.9\"",
                "\"name\": \"PAPERSIZE\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"9.8\"",
                "\"name\": \"TOPMARGIN\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"120.75\"",
                "\"name\": \"BOTMARGIN\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"240.25\"",
                "\"name\": \"GRIDV\"",
                "\"sourceLineIndex\": 4",
                "\"value\": \"1.1\"",
                "\"name\": \"GRIDH\"",
                "\"sourceLineIndex\": 5",
                "\"value\": \"0.9\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 6",
                "\"value\": \"3.5\"",
                "\"name\": \"COLWIDTH\"",
                "\"sourceLineIndex\": 7",
                "\"value\": \"5000.99\"",
                "\"name\": \"COLSPACING\"",
                "\"sourceLineIndex\": 8",
                "\"value\": \"42.42\""
            },
            "#1751: fractional live settings memo selection should expose fractional source values");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1751: fractional deleted settings memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1751: fractional deleted settings memo selection should expose raw settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1751: fractional deleted settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2325: selected fractional deleted settings memo JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 2",
                "\"value\": \"2.1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"10.9\"",
                "\"name\": \"TOPMARGIN\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"360.5\"",
                "\"name\": \"BOTMARGIN\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"480.5\"",
                "\"name\": \"GRIDV\"",
                "\"sourceLineIndex\": 4",
                "\"value\": \"0.1\"",
                "\"name\": \"GRIDH\"",
                "\"sourceLineIndex\": 5",
                "\"value\": \"1.1\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 6",
                "\"value\": \"4.9\"",
                "\"name\": \"COLWIDTH\"",
                "\"sourceLineIndex\": 7",
                "\"value\": \"6000.5\"",
                "\"name\": \"COLSPACING\"",
                "\"sourceLineIndex\": 8",
                "\"value\": \"84.9\""
            },
            "#1751: fractional deleted settings memo selection should expose fractional source values");
    };

    run_fractional_setting_memo_layout(temp_root / "fractional_setting_memo.frx",
                                       "fractional_setting_memo.frx",
                                       "report");
    run_fractional_setting_memo_layout(temp_root / "fractional_setting_memo.lbx",
                                       "fractional_setting_memo.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_blank_report_setting_memo_values(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_blank_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_blank_setting_memo_layout = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_blank_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " blank setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " blank setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1752: blank settings memo values should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1752: blank settings memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1752: blank settings memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 9",
                        "#1752: blank settings memo values should preserve live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 9",
                        "#1752: blank settings memo values should preserve deleted raw settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1752: blank settings memo values should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1752: blank memo orientation should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1752: blank memo paper size should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": false",
                        "#1752: blank memo top margin should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"bottomMarginAvailable\": false",
                        "#1752: blank memo bottom margin should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": false",
                        "#1752: blank memo vertical grid should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": false",
                        "#1752: blank memo horizontal grid should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1752: blank settings memo values should not fabricate column setup");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1752: blank memo column count should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnWidthAvailable\": false",
                        "#1752: blank memo column width should not advertise availability");
        expect_contains(summary_process.stdout_text, "\"columnSpacingAvailable\": false",
                        "#1752: blank memo column spacing should not advertise availability");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"\"",
                        "#1752: live blank memo orientation provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLSPACING\", \"recordIndex\": 1, \"fieldIndex\": 2, \"sourceLineIndex\": 8, \"memoBlockNumber\": 2, \"value\": \"\"",
                        "#1752: deleted blank memo column-spacing provenance should remain inspectable");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2326: blank settings memo summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1752: blank live settings memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1752: blank live settings memo selection should expose raw settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1752: blank live settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2326: selected blank live settings memo JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 1",
                "\"value\": \"\"",
                "\"name\": \"PAPERSIZE\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"\"",
                "\"name\": \"TOPMARGIN\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"\"",
                "\"name\": \"BOTMARGIN\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"\"",
                "\"name\": \"GRIDV\"",
                "\"sourceLineIndex\": 4",
                "\"value\": \"\"",
                "\"name\": \"GRIDH\"",
                "\"sourceLineIndex\": 5",
                "\"value\": \"\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 6",
                "\"value\": \"\"",
                "\"name\": \"COLWIDTH\"",
                "\"sourceLineIndex\": 7",
                "\"value\": \"\"",
                "\"name\": \"COLSPACING\"",
                "\"sourceLineIndex\": 8",
                "\"value\": \"\""
            },
            "#1752: blank live settings memo selection should expose blank source values");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1752: blank deleted settings memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1752: blank deleted settings memo selection should expose raw settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1752: blank deleted settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2326: selected blank deleted settings memo JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 2",
                "\"value\": \"\"",
                "\"name\": \"PAPERSIZE\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"\"",
                "\"name\": \"TOPMARGIN\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"\"",
                "\"name\": \"BOTMARGIN\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"\"",
                "\"name\": \"GRIDV\"",
                "\"sourceLineIndex\": 4",
                "\"value\": \"\"",
                "\"name\": \"GRIDH\"",
                "\"sourceLineIndex\": 5",
                "\"value\": \"\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 6",
                "\"value\": \"\"",
                "\"name\": \"COLWIDTH\"",
                "\"sourceLineIndex\": 7",
                "\"value\": \"\"",
                "\"name\": \"COLSPACING\"",
                "\"sourceLineIndex\": 8",
                "\"value\": \"\""
            },
            "#1752: blank deleted settings memo selection should expose blank source values");
    };

    run_blank_setting_memo_layout(temp_root / "blank_setting_memo.frx",
                                  "blank_setting_memo.frx",
                                  "report");
    run_blank_setting_memo_layout(temp_root / "blank_setting_memo.lbx",
                                  "blank_setting_memo.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_malformed_report_setting_memo_lines(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_malformed_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_malformed_setting_memo_layout = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_malformed_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " malformed setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " malformed setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1753: malformed settings memo lines should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1753: malformed settings memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1753: malformed settings memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 3",
                        "#1753: malformed settings memo lines should preserve only valid live settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#1753: malformed settings memo lines should preserve only valid deleted settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1753: valid live memo page settings should still expose page setup");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1753: valid live memo orientation should still parse");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1753: valid live memo paper size should still parse");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1753: valid live memo column settings should still expose column setup");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1753: valid live memo column count should still parse");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2, \"memoBlockNumber\": 1, \"value\": \"1\"",
                        "#1753: live orientation should retain original source-line index after skipped lines");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4, \"memoBlockNumber\": 1, \"value\": \"9\"",
                        "#1753: live paper-size should retain original source-line index after skipped lines");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLS\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 6, \"memoBlockNumber\": 1, \"value\": \"3\"",
                        "#1753: live column-count should retain original source-line index after skipped lines");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLSPACING\", \"recordIndex\": 1, \"fieldIndex\": 2, \"sourceLineIndex\": 7, \"memoBlockNumber\": 2, \"value\": \"42\"",
                        "#1753: deleted column-spacing should retain original source-line index after skipped lines");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2327: malformed settings memo summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1753: malformed live settings memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1753: malformed live settings memo selection should expose valid settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1753: malformed live settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2327: selected malformed live settings memo JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 2",
                "\"memoBlockNumber\": 1",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"sourceLineIndex\": 4",
                "\"value\": \"9\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 6",
                "\"value\": \"3\""
            },
            "#1753: malformed live settings memo selection should expose only valid memo settings");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1753: malformed deleted settings memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1753: malformed deleted settings memo selection should expose valid settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1753: malformed deleted settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2327: selected malformed deleted settings memo JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 2",
                "\"memoBlockNumber\": 2",
                "\"value\": \"120\"",
                "\"name\": \"BOTMARGIN\"",
                "\"sourceLineIndex\": 4",
                "\"value\": \"240\"",
                "\"name\": \"COLWIDTH\"",
                "\"sourceLineIndex\": 6",
                "\"value\": \"5000\"",
                "\"name\": \"COLSPACING\"",
                "\"sourceLineIndex\": 7",
                "\"value\": \"42\""
            },
            "#1753: malformed deleted settings memo selection should expose only valid memo settings");
    };

    run_malformed_setting_memo_layout(temp_root / "malformed_setting_memo.frx",
                                      "malformed_setting_memo.frx",
                                      "report");
    run_malformed_setting_memo_layout(temp_root / "malformed_setting_memo.lbx",
                                      "malformed_setting_memo.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_parses_cr_only_report_setting_memo_lines(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_cr_only_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_cr_only_setting_memo_layout = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_cr_only_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " CR-only setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " CR-only setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1756: CR-only settings memo lines should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1756: CR-only settings memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1756: CR-only settings memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 4",
                        "#1756: CR-only settings memo lines should preserve separate live settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1756: CR-only settings memo lines should preserve separate deleted settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1756: CR-only memo page settings should expose page setup");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1756: CR-only memo orientation should parse");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1756: CR-only memo paper size should parse");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1756: CR-only memo column settings should expose column setup");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1756: CR-only memo column count should parse");
        expect_contains(summary_process.stdout_text, "\"columnWidth\": 5000",
                        "#1756: CR-only memo column width should parse");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"1\"",
                        "#1756: CR-only live orientation should retain line-zero provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"9\"",
                        "#1756: CR-only live paper-size should retain source-line provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3, \"memoBlockNumber\": 1, \"value\": \"5000\"",
                        "#1756: CR-only live column-width should retain source-line provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLSPACING\", \"recordIndex\": 1, \"fieldIndex\": 2, \"sourceLineIndex\": 2, \"memoBlockNumber\": 2, \"value\": \"42\"",
                        "#1756: CR-only deleted column-spacing should retain source-line provenance");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2330: CR-only settings memo summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1756: CR-only live settings memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1756: CR-only live settings memo selection should expose settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1756: CR-only live settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2330: selected CR-only live settings memo JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 1",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"9\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"3\"",
                "\"name\": \"COLWIDTH\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"5000\""
            },
            "#1756: CR-only live settings memo selection should expose separate memo settings");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1756: CR-only deleted settings memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1756: CR-only deleted settings memo selection should expose settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1756: CR-only deleted settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2330: selected CR-only deleted settings memo JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 2",
                "\"value\": \"120\"",
                "\"name\": \"BOTMARGIN\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"240\"",
                "\"name\": \"COLSPACING\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"42\""
            },
            "#1756: CR-only deleted settings memo selection should expose separate memo settings");
    };

    run_cr_only_setting_memo_layout(temp_root / "cr_only_setting_memo.frx",
                                    "cr_only_setting_memo.frx",
                                    "report");
    run_cr_only_setting_memo_layout(temp_root / "cr_only_setting_memo.lbx",
                                    "cr_only_setting_memo.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_parses_mixed_case_report_setting_memo_names(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_mixed_case_setting_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_mixed_case_setting_memo_layout = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_mixed_case_setting_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " mixed-case setting memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " mixed-case setting memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1757: mixed-case settings memo names should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1757: mixed-case settings memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1757: mixed-case settings memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 5",
                        "#1757: mixed-case settings memo names should preserve live settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 4",
                        "#1757: mixed-case settings memo names should preserve deleted settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1757: mixed-case memo page settings should expose page setup");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1757: lower-case memo orientation should parse case-insensitively");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1757: mixed-case memo paper size should parse case-insensitively");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1757: mixed-case memo top margin should parse case-insensitively");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1757: mixed-case memo column settings should expose column setup");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1757: lower-case memo column count should parse case-insensitively");
        expect_contains(summary_process.stdout_text, "\"columnWidth\": 5000",
                        "#1757: mixed-case memo column width should parse case-insensitively");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"orientation\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"1\"",
                        "#1757: lower-case live orientation should preserve source spelling and provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"PaperSize\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"9\"",
                        "#1757: mixed-case live paper-size should preserve source spelling and provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ColSpacing\", \"recordIndex\": 1, \"fieldIndex\": 2, \"sourceLineIndex\": 3, \"memoBlockNumber\": 2, \"value\": \"42\"",
                        "#1757: mixed-case deleted column-spacing should preserve source spelling and provenance");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2331: mixed-case settings memo summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1757: mixed-case live settings memo selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1757: mixed-case live settings memo selection should expose settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1757: mixed-case live settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2331: selected mixed-case live settings memo JSON");
        expect_contains_in_order(
            live_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"orientation\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 1",
                "\"value\": \"1\"",
                "\"name\": \"PaperSize\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"9\"",
                "\"name\": \"TopMargin\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"120\"",
                "\"name\": \"cols\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"3\"",
                "\"name\": \"ColWidth\"",
                "\"sourceLineIndex\": 4",
                "\"value\": \"5000\""
            },
            "#1757: mixed-case live settings memo selection should preserve source spelling");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1757: mixed-case deleted settings memo selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1757: mixed-case deleted settings memo selection should expose settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1757: mixed-case deleted settings memo selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2331: selected mixed-case deleted settings memo JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"bottommargin\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 2",
                "\"value\": \"240\"",
                "\"name\": \"GridV\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"1\"",
                "\"name\": \"gridh\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"0\"",
                "\"name\": \"ColSpacing\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"42\""
            },
            "#1757: mixed-case deleted settings memo selection should preserve source spelling");
    };

    run_mixed_case_setting_memo_layout(temp_root / "mixed_case_setting_memo.frx",
                                       "mixed_case_setting_memo.frx",
                                       "report");
    run_mixed_case_setting_memo_layout(temp_root / "mixed_case_setting_memo.lbx",
                                       "mixed_case_setting_memo.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

}  // namespace cf_test_studio_host_json
