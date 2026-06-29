#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
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

}  // namespace cf_test_studio_host_json
