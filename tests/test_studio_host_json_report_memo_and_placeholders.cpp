// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_unresolved_memo_placeholder_layout_json(
    const std::filesystem::path& report_path) {
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
        {.name = "TOPMARGIN", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "<memo block 30>", "", "", "", "", "", "", "", "<memo block 31>"},
        {"1", "53", "<memo block 32>", "", "", "", "", "", "", "", "<memo block 33>"},
        {"9", "4", "", "", "2000", "", "5000", "", "", "", ""},
        {"8", "0", "<memo block 34>", "1200", "2600", "4000", "450",
         "<memo block 35>", "<memo block 36>", "<memo block 37>", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1736: synthetic report table with unresolved memo placeholders should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1736: synthetic report table should mark unresolved memo settings deleted");
}

#if !defined(COPPERFIN_REPORT_UNRESOLVED_MEMO_SKIP_HOST_SMOKE)
void test_studio_host_json_suppresses_unresolved_report_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_report_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_unresolved_memo_layout = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_unresolved_memo_placeholder_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1736: unresolved report/label memo placeholders should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1736: unresolved memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1736: unresolved memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"settingCount\": 0",
                        "#1736: unresolved root memo placeholders should not become live settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1736: unresolved deleted root memo placeholders should not become deleted settings");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1736: unresolved root memo placeholders should not fabricate page setup");
        expect_contains(summary_process.stdout_text, "\"highlightCount\": 0",
                        "#1736: unresolved object memo placeholders should not become highlights");
        expect_unresolved_memo_preview_bounds(
            summary_process.stdout_text,
            "#2332: unresolved memo placeholder summary JSON");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1736: unresolved memo placeholders should not leak into summary JSON");

        const auto live_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_settings_process.exit_code == 0,
               "#1736: unresolved live root settings selection should keep inspection non-failing");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1736: unresolved live root settings should not expose selected settings");
        expect_contains(live_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1736: unresolved live root selected settings should be null");
        expect_unresolved_memo_preview_bounds(
            live_settings_process.stdout_text,
            "#2332: selected unresolved live root memo JSON");
        expect_not_contains(live_settings_process.stdout_text, "<memo block",
                            "#1736: unresolved live root memo placeholders should not leak into selection JSON");

        const auto deleted_settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_settings_process.exit_code == 0,
               "#1736: unresolved deleted root settings selection should keep inspection non-failing");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1736: unresolved deleted root settings should not expose selected settings");
        expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1736: unresolved deleted root selected settings should be null");
        expect_unresolved_memo_preview_bounds(
            deleted_settings_process.stdout_text,
            "#2332: selected unresolved deleted root memo JSON");
        expect_not_contains(deleted_settings_process.stdout_text, "<memo block",
                            "#1736: unresolved deleted root memo placeholders should not leak into selection JSON");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        expect(object_process.exit_code == 0,
               "#1736: unresolved object memo selection should keep inspection non-failing");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1736: unresolved object memo selection should still select the layout object");
        expect_contains(object_process.stdout_text, "\"expression\": \"\"",
                        "#1736: unresolved object expressions should be suppressed");
        expect_contains(object_process.stdout_text, "\"highlightCount\": 0",
                        "#1736: unresolved font/mode memo placeholders should not become selected-object highlights");
        expect_unresolved_memo_preview_bounds(
            object_process.stdout_text,
            "#2332: selected unresolved object memo JSON");
        expect_not_contains(object_process.stdout_text, "<memo block",
                            "#1736: unresolved object memo placeholders should not leak into selected-object JSON");
    };

    run_unresolved_memo_layout(temp_root / "unresolved_memo.frx",
                               "unresolved_memo.frx",
                               "report");
    run_unresolved_memo_layout(temp_root / "unresolved_memo.lbx",
                               "unresolved_memo.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

}  // namespace cf_test_studio_host_json
