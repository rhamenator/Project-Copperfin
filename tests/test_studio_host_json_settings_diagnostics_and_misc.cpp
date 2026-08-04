// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_duplicate_setting_precedence_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "ORIENTATIO", .type = 'C', .length = 24U},
        {.name = "COLS", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53",
         "ORIENTATION=1\n"
         "ORIENTATION=2\n"
         "COLS=3\n"
         "COLS=4",
         "9", "8", "duplicate-precedence-live-settings-guid"},
        {"1", "53",
         "ORIENTATION=5\n"
         "ORIENTATION=6\n"
         "COLS=7\n"
         "COLS=8",
         "10", "11", "duplicate-precedence-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1754: synthetic report table with duplicate settings should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1754: synthetic report table should mark duplicate settings deleted");
}

void write_synthetic_report_table_for_live_deleted_ambiguous_settings_json(
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
        {"1", "53", "ORIENTATION=0", "", "", "duplicate-live-deleted-guid"},
        {"1", "53", "PAPERSIZE=1", "", "", "DUPLICATE-LIVE-DELETED-GUID"},
        {"9", "4", "", "0", "3200", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1702: synthetic report table for live/deleted ambiguous settings JSON should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1702: synthetic report table should mark duplicate settings deleted");
}

void write_synthetic_report_table_for_padded_stable_settings_json(
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
        {"1", "53", "ORIENTATION=0", "", "", "  padded-settings-guid  "},
        {"9", "4", "", "0", "3200", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1703: synthetic report table for padded stable settings JSON should be created");
}

void write_synthetic_report_table_for_deep_stable_settings_json(
    const std::filesystem::path& report_path) {
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

void write_synthetic_report_table_for_deep_ambiguous_stable_settings_json(
    const std::filesystem::path& report_path) {
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

void write_synthetic_report_table_for_deep_live_deleted_ambiguous_stable_settings_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deep_ambiguous_stable_settings_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 10U, true);
    expect(delete_result.ok,
           "#1709: synthetic report table should mark the deep duplicate settings row deleted");
}

void write_synthetic_report_table_for_deleted_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#1476: synthetic FRX table should mark report settings deleted");
}

#if !defined(COPPERFIN_REPORT_SETTINGS_DIAGNOSTICS_SKIP_HOST_SMOKE)
void test_studio_host_json_deletes_report_settings_by_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_settings_delete_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", report_path.string(),
            "--delete-object",
            "--record", "0",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host report settings delete stdout:\n" << delete_process.stdout_text << "\n";
        std::cerr << "studio host report settings delete stderr:\n" << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0,
           "#1475: report settings delete should exit successfully");
    expect(dbf_record_deleted(report_path, 0U),
           "#1475: report settings delete should mark the FRX settings record deleted");
    expect_full_report_layout_preview_bounds(
        delete_process.stdout_text,
        "#2040: record-selected report settings delete JSON");
    expect_contains(delete_process.stdout_text, "\"settingCount\": 0",
                    "#1475: deleted report settings JSON should remove settings from live counts");
    expect_contains(delete_process.stdout_text, "\"pageSetupAvailable\": true",
                    "#3815: deleted report settings JSON should expose effective page setup summaries");
    expect_contains(delete_process.stdout_text, "\"orientationAvailable\": true",
                    "#3815: deleted report settings JSON should expose effective orientation availability");
    expect_contains(delete_process.stdout_text, "\"orientationCode\": 0",
                    "#3815: deleted report settings JSON should expose the effective orientation code");
    expect_contains(delete_process.stdout_text, "\"paperSizeAvailable\": true",
                    "#3815: deleted report settings JSON should expose effective paper-size availability");
    expect_contains(delete_process.stdout_text, "\"paperSizeCode\": 1",
                    "#3815: deleted report settings JSON should expose the effective paper-size code");
    expect_contains(delete_process.stdout_text, "\"topMargin\": 10",
                    "#3815: deleted report settings JSON should expose the effective top margin");
    expect_contains(delete_process.stdout_text, "\"bottomMargin\": 20",
                    "#3815: deleted report settings JSON should expose the effective bottom margin");
    expect_contains(delete_process.stdout_text, "\"gridVertical\": 4",
                    "#3815: deleted report settings JSON should expose effective vertical-grid spacing");
    expect_contains(delete_process.stdout_text, "\"gridHorizontal\": 8",
                    "#3815: deleted report settings JSON should expose effective horizontal-grid spacing");
    expect_contains(delete_process.stdout_text, "\"deletedSettingCount\": 6",
                    "#1475: deleted report settings JSON should expose deleted setting counts");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"deletedSettings\": [",
            "\"name\": \"ORIENTATION\"",
            "\"recordIndex\": 0",
            "\"name\": \"PAPERSIZE\"",
            "\"recordIndex\": 0",
            "\"name\": \"BOTMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDV\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDH\"",
            "\"recordIndex\": 0",
            "\"name\": \"TOPMARGIN\"",
            "\"recordIndex\": 0"
        },
        "#1475: report layout JSON should move root settings into deleted-setting metadata");
    expect_contains(delete_process.stdout_text, "\"sectionCount\": 2",
                    "#1475: deleting report settings should preserve live section metadata");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1475: deleting report settings should preserve deleted object metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_report_settings_without_root_objcode_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_root_objcode_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_missing_root_objcode_layout = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_missing_root_objcode_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing root OBJCODE summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing root OBJCODE summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1730: missing root OBJCODE schema should keep report/label settings inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1730: missing root OBJCODE layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1730: missing root OBJCODE label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1730: missing root OBJCODE should preserve live page setup availability");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": true",
                        "#1730: missing root OBJCODE should preserve live orientation availability");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1730: missing root OBJCODE should preserve live orientation codes");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": true",
                        "#1730: missing root OBJCODE should preserve live paper-size availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1730: missing root OBJCODE should preserve live paper-size codes");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": true",
                        "#1730: missing root OBJCODE should preserve live top-margin availability");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1730: missing root OBJCODE should preserve live top margins");
        expect_contains(summary_process.stdout_text, "\"gridVerticalAvailable\": true",
                        "#1730: missing root OBJCODE should preserve EXPR-derived vertical grid availability");
        expect_contains(summary_process.stdout_text, "\"gridVertical\": 1",
                        "#1730: missing root OBJCODE should preserve EXPR-derived vertical grid values");
        expect_contains(summary_process.stdout_text, "\"gridHorizontalAvailable\": true",
                        "#1730: missing root OBJCODE should preserve EXPR-derived horizontal grid availability");
        expect_contains(summary_process.stdout_text, "\"gridHorizontal\": 0",
                        "#1730: missing root OBJCODE should preserve EXPR-derived horizontal grid values");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 5",
                        "#1730: missing root OBJCODE layouts should preserve live setting counts");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 5",
                        "#1730: missing root OBJCODE layouts should preserve deleted setting counts");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 1, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"1\"",
                        "#1730: live EXPR-derived setting provenance should remain available without OBJCODE");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 1, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"0\"",
                        "#1730: live second-line EXPR setting provenance should remain available without OBJCODE");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"1\"",
                        "#1730: live direct orientation provenance should remain available without OBJCODE");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLS\", \"recordIndex\": 1, \"fieldIndex\": 1, \"sourceLineIndex\": 0, \"memoBlockNumber\": 2, \"value\": \"3\"",
                        "#1730: deleted EXPR-derived setting provenance should remain available without OBJCODE");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"TOPMARGIN\", \"recordIndex\": 1, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"240\"",
                        "#1730: deleted direct top-margin provenance should remain available without OBJCODE");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2317: missing root OBJCODE settings summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1730: missing root OBJCODE live settings selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1730: missing root OBJCODE live settings should advertise selected-settings availability");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1730: missing root OBJCODE live settings should expose settings selection kind");
        expect_empty_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2317: selected missing root OBJCODE live settings JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"GRIDV\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 1",
                "\"sourceLineIndex\": 0",
                "\"value\": \"1\"",
                "\"name\": \"GRIDH\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 1",
                "\"sourceLineIndex\": 1",
                "\"value\": \"0\"",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 3",
                "\"value\": \"9\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"120\""
            },
            "#1730: missing root OBJCODE live selection should expose selected-settings metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1730: missing root OBJCODE deleted settings selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1730: missing root OBJCODE deleted settings should advertise selected-settings availability");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1730: missing root OBJCODE deleted settings should expose settings selection kind");
        expect_empty_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2317: selected missing root OBJCODE deleted settings JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"COLS\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 1",
                "\"sourceLineIndex\": 0",
                "\"value\": \"3\"",
                "\"name\": \"COLWIDTH\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 1",
                "\"sourceLineIndex\": 1",
                "\"value\": \"5000\"",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"value\": \"0\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"1\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 4",
                "\"value\": \"240\""
            },
            "#1730: missing root OBJCODE deleted selection should expose selected-settings metadata");
    };

    run_missing_root_objcode_layout(temp_root / "missing_root_objcode.frx",
                                    "missing_root_objcode.frx",
                                    "report");
    run_missing_root_objcode_layout(temp_root / "missing_root_objcode.lbx",
                                    "missing_root_objcode.lbx",
                                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_duplicate_report_setting_precedence(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_duplicate_setting_precedence_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_duplicate_setting_precedence_layout = [&](const fs::path& asset_path,
                                                             const std::string& title,
                                                             const std::string& label) {
        write_synthetic_report_table_for_duplicate_setting_precedence_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " duplicate setting precedence summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " duplicate setting precedence summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1754: duplicate settings should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1754: duplicate setting layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1754: duplicate setting label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 6",
                        "#1754: duplicate settings should keep all live setting entries inspectable");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1754: duplicate settings should keep all deleted setting entries inspectable");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1754: first live memo orientation should expose page setup");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1754: first live memo orientation should beat later duplicates");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": true",
                        "#1754: first live memo column setting should expose column setup");
        expect_contains(summary_process.stdout_text, "\"columnCount\": 3",
                        "#1754: first live memo column count should beat later duplicates");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"1\"",
                        "#1754: first live memo orientation should retain provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"2\"",
                        "#1754: later live memo orientation should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"9\"",
                        "#1754: direct live orientation duplicate should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLS\", \"recordIndex\": 1, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"11\"",
                        "#1754: direct deleted column duplicate should remain inspectable");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2328: duplicate settings summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1754: duplicate live settings selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1754: duplicate live settings selection should expose settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1754: duplicate live settings selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2328: selected duplicate live settings JSON");
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
                "\"name\": \"ORIENTATION\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"2\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"3\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"4\"",
                "\"name\": \"ORIENTATION\"",
                "\"fieldIndex\": 3",
                "\"sourceLineIndex\": null",
                "\"value\": \"9\"",
                "\"name\": \"COLS\"",
                "\"fieldIndex\": 4",
                "\"sourceLineIndex\": null",
                "\"value\": \"8\""
            },
            "#1754: duplicate live settings selection should expose memo entries before direct entries");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1754: duplicate deleted settings selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1754: duplicate deleted settings selection should expose settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1754: duplicate deleted settings selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2328: selected duplicate deleted settings JSON");
        expect_contains_in_order(
            deleted_settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"sourceLineIndex\": 0",
                "\"memoBlockNumber\": 2",
                "\"value\": \"5\"",
                "\"name\": \"ORIENTATION\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"6\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"7\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"8\"",
                "\"name\": \"ORIENTATION\"",
                "\"fieldIndex\": 3",
                "\"sourceLineIndex\": null",
                "\"value\": \"10\"",
                "\"name\": \"COLS\"",
                "\"fieldIndex\": 4",
                "\"sourceLineIndex\": null",
                "\"value\": \"11\""
            },
            "#1754: duplicate deleted settings selection should expose memo entries before direct entries");
    };

    run_duplicate_setting_precedence_layout(temp_root / "duplicate_setting_precedence.frx",
                                            "duplicate_setting_precedence.frx",
                                            "report");
    run_duplicate_setting_precedence_layout(temp_root / "duplicate_setting_precedence.lbx",
                                            "duplicate_setting_precedence.lbx",
                                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_invalid_first_duplicate_report_setting_precedence(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_invalid_first_duplicate_setting_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_invalid_first_duplicate_setting_layout = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_synthetic_report_table_for_invalid_first_duplicate_setting_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " invalid-first duplicate setting summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " invalid-first duplicate setting summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1755: invalid-first duplicate settings should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1755: invalid-first duplicate setting layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1755: invalid-first duplicate setting label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 6",
                        "#1755: invalid-first duplicate settings should keep all live setting entries inspectable");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 6",
                        "#1755: invalid-first duplicate settings should keep all deleted setting entries inspectable");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1755: invalid first duplicate orientation should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1755: later valid duplicate orientation should not override invalid first match");
        expect_contains(summary_process.stdout_text, "\"columnSetupAvailable\": false",
                        "#1755: invalid first duplicate column setting should not fabricate column setup");
        expect_contains(summary_process.stdout_text, "\"columnCountAvailable\": false",
                        "#1755: later valid duplicate column count should not override invalid first match");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"sideways\"",
                        "#1755: invalid first live memo orientation should retain provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"1\"",
                        "#1755: later valid live memo orientation should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"9\"",
                        "#1755: direct valid live orientation duplicate should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLS\", \"recordIndex\": 1, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"11\"",
                        "#1755: direct valid deleted column duplicate should remain inspectable");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2329: invalid-first duplicate settings summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1755: invalid-first duplicate live settings selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1755: invalid-first duplicate live settings selection should expose settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1755: invalid-first duplicate live settings selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            live_settings_process.stdout_text,
            "#2329: selected invalid-first duplicate live settings JSON");
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
                "\"name\": \"ORIENTATION\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"1\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"many\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"3\"",
                "\"name\": \"ORIENTATION\"",
                "\"fieldIndex\": 3",
                "\"sourceLineIndex\": null",
                "\"value\": \"9\"",
                "\"name\": \"COLS\"",
                "\"fieldIndex\": 4",
                "\"sourceLineIndex\": null",
                "\"value\": \"8\""
            },
            "#1755: invalid-first duplicate live selection should expose memo entries before direct entries");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1755: invalid-first duplicate deleted settings selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1755: invalid-first duplicate deleted settings selection should expose settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1755: invalid-first duplicate deleted settings selection should expose settings kind");
        expect_empty_report_layout_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2329: selected invalid-first duplicate deleted settings JSON");
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
                "\"name\": \"ORIENTATION\"",
                "\"sourceLineIndex\": 1",
                "\"value\": \"5\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 2",
                "\"value\": \"deleted-many\"",
                "\"name\": \"COLS\"",
                "\"sourceLineIndex\": 3",
                "\"value\": \"7\"",
                "\"name\": \"ORIENTATION\"",
                "\"fieldIndex\": 3",
                "\"sourceLineIndex\": null",
                "\"value\": \"10\"",
                "\"name\": \"COLS\"",
                "\"fieldIndex\": 4",
                "\"sourceLineIndex\": null",
                "\"value\": \"11\""
            },
            "#1755: invalid-first duplicate deleted selection should expose memo entries before direct entries");
    };

    run_invalid_first_duplicate_setting_layout(temp_root / "invalid_first_duplicate_setting.frx",
                                               "invalid_first_duplicate_setting.frx",
                                               "report");
    run_invalid_first_duplicate_setting_layout(temp_root / "invalid_first_duplicate_setting.lbx",
                                               "invalid_first_duplicate_setting.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_preserves_report_settings_without_root_expr_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_root_expr_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_missing_root_expr_layout = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_missing_root_expr_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing root EXPR summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing root EXPR summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1723: missing root EXPR schema should keep report/label settings inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1723: missing root EXPR layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1723: missing root EXPR label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1723: direct settings should preserve live page setup availability without EXPR");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": true",
                        "#1723: direct orientation should remain available without EXPR");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1723: direct orientation code should be preserved without EXPR");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": true",
                        "#1723: direct paper size should remain available without EXPR");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1723: direct paper size code should be preserved without EXPR");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": true",
                        "#1723: direct top margin should remain available without EXPR");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 120",
                        "#1723: direct top margin should be preserved without EXPR");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 3",
                        "#1723: missing root EXPR layouts should preserve live direct setting counts");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1723: missing root EXPR layouts should preserve deleted direct setting counts");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"1\"",
                        "#1723: live direct orientation provenance should use the direct field without EXPR");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"9\"",
                        "#1723: live direct paper-size provenance should use the direct field without EXPR");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"120\"",
                        "#1723: live direct top-margin provenance should use the direct field without EXPR");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 1, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"0\"",
                        "#1723: deleted direct orientation provenance should use the direct field without EXPR");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2337: missing root EXPR settings summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1723: missing root EXPR live settings selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1723: missing root EXPR live settings should advertise selected-settings availability");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1723: missing root EXPR live settings should expose settings selection kind");
        expect_empty_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2337: selected missing root EXPR live settings JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 3",
                "\"value\": \"9\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"120\""
            },
            "#1723: missing root EXPR live selection should expose direct selected-settings metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1723: missing root EXPR deleted settings selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1723: missing root EXPR deleted settings should advertise selected-settings availability");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1723: missing root EXPR deleted settings should expose settings selection kind");
        expect_empty_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2337: selected missing root EXPR deleted settings JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"value\": \"0\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"1\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 4",
                "\"value\": \"240\""
            },
            "#1723: missing root EXPR deleted selection should expose direct selected-settings metadata");
    };

    run_missing_root_expr_layout(temp_root / "missing_root_expr.frx",
                                 "missing_root_expr.frx",
                                 "report");
    run_missing_root_expr_layout(temp_root / "missing_root_expr.lbx",
                                 "missing_root_expr.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_printer_identity_report_settings_summary(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_printer_identity_settings_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_printer_identity_layout = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_synthetic_report_table_for_printer_identity_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " printer identity summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " printer identity summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#3796: printer-identity settings summary should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3796: printer-identity layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#3796: printer-identity label layouts should retain label identity");
        } else {
            expect_contains(summary_process.stdout_text, "\"isLabel\": false",
                            "#3796: printer-identity report layouts should retain report identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3796: printer-identity layouts should mark page setup available");
        expect_contains(summary_process.stdout_text, "\"driverAvailable\": true",
                        "#3796: printer-identity layouts should expose driver availability");
        expect_contains(summary_process.stdout_text, "\"driver\": \"cups\"",
                        "#3796: printer-identity layouts should expose the live driver");
        expect_contains(summary_process.stdout_text, "\"deviceAvailable\": true",
                        "#3796: printer-identity layouts should expose device availability");
        expect_contains(summary_process.stdout_text, "\"device\": \"HP LaserJet\"",
                        "#3796: printer-identity layouts should expose the live device");
        expect_contains(summary_process.stdout_text, "\"outputAvailable\": true",
                        "#3796: printer-identity layouts should expose output availability");
        expect_contains(summary_process.stdout_text, "\"output\": \"LPT1\"",
                        "#3796: printer-identity layouts should expose the live output");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 3",
                        "#3796: printer-identity layouts should preserve live setting counts");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#3796: printer-identity layouts should preserve deleted setting counts");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"DRIVER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"cups\"",
                        "#3796: printer-identity layouts should preserve live driver provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"DEVICE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"HP LaserJet\"",
                        "#3796: printer-identity layouts should preserve live device provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"OUTPUT\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2, \"memoBlockNumber\": 1, \"value\": \"LPT1\"",
                        "#3796: printer-identity layouts should preserve live output provenance");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#3796: printer-identity settings summary JSON");
    };

    run_printer_identity_layout(temp_root / "printer_identity.frx",
                                "printer_identity.frx",
                                "report");
    run_printer_identity_layout(temp_root / "printer_identity.lbx",
                                "printer_identity.lbx",
                                "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_color_and_copies_report_settings_summary(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_color_copies_settings_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto write_color_copies_layout = [](const fs::path& asset_path) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 10U},
            {.name = "OBJCODE", .type = 'N', .length = 10U},
            {.name = "EXPR", .type = 'M', .length = 10U}
        };
        const std::vector<std::vector<std::string>> records{
            {"1", "53", "ORIENTATION=0\r\nPAPERSIZE=1\r\nGRIDV=4\r\nGRIDH=8\r\nCOLOR=0\r\nCOPIES=3"}
        };

        const auto create_result =
            copperfin::vfp::create_dbf_table_file(asset_path.string(), fields, records);
        expect(create_result.ok,
               "#3806: synthetic report table for color/copies settings summary should be created");
    };

    const auto run_color_copies_layout = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_color_copies_layout(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " color/copies summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " color/copies summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#3806: color/copies settings summary should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#3806: color/copies layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#3806: color/copies label layouts should retain label identity");
        } else {
            expect_contains(summary_process.stdout_text, "\"isLabel\": false",
                            "#3806: color/copies report layouts should retain report identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#3806: color/copies layouts should mark page setup available");
        expect_contains(summary_process.stdout_text, "\"colorAvailable\": true",
                        "#3806: color/copies layouts should expose color availability");
        expect_contains(summary_process.stdout_text, "\"color\": 0",
                        "#3806: color/copies layouts should expose the live color value");
        expect_contains(summary_process.stdout_text, "\"copiesAvailable\": true",
                        "#3806: color/copies layouts should expose copies availability");
        expect_contains(summary_process.stdout_text, "\"copies\": 3",
                        "#3806: color/copies layouts should expose the live copies value");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 6",
                        "#3806: color/copies layouts should preserve live setting counts");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4, \"memoBlockNumber\": 1, \"value\": \"0\"",
                        "#3806: color/copies layouts should preserve COLOR provenance");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"COPIES\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5, \"memoBlockNumber\": 1, \"value\": \"3\"",
                        "#3806: color/copies layouts should preserve COPIES provenance");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#3806: color/copies settings summary JSON");
    };

    run_color_copies_layout(temp_root / "color_copies.frx",
                            "color_copies.frx",
                            "report");
    run_color_copies_layout(temp_root / "color_copies.lbx",
                            "color_copies.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

}  // namespace cf_test_studio_host_json
