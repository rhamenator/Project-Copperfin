// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_locale_catalog_environment_support.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace {

int failures = 0;
using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) != std::string::npos, message);
}

void expect_contains_in_order(
    const std::string& text,
    const std::vector<std::string>& needles,
    const std::string& message) {
    std::size_t offset = 0U;
    for (const auto& needle : needles) {
        const std::size_t position = text.find(needle, offset);
        if (position == std::string::npos) {
            expect(false, message);
            return;
        }
        offset = position + needle.size();
    }
}

bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index) {
    const auto table_result =
        copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return false;
    }
    return table_result.table.records[record_index].deleted;
}

std::string quote_command_argument(const std::string& value) {
    std::string quoted = "\"";
    quoted.reserve(value.size() + 2U);
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('"');
    return quoted;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
    std::string stderr_text;
};

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    namespace fs = std::filesystem;

    const fs::path resolved_executable_path = fs::absolute(executable_path);
    const fs::path stdout_path = working_directory / "studio_host_stdout.log";
    const fs::path stderr_path = working_directory / "studio_host_stderr.log";

    std::string command = quote_command_argument(resolved_executable_path.string());
    for (const auto& argument : arguments) {
        command += " ";
        command += quote_command_argument(argument);
    }
    command += " > ";
    command += quote_command_argument(stdout_path.string());
    command += " 2> ";
    command += quote_command_argument(stderr_path.string());

    const fs::path original_directory = fs::current_path();
    fs::current_path(working_directory);
    const int raw_exit_code = copperfin::test_support::run_shell_command(command);
    fs::current_path(original_directory);

    ProcessResult result;
    if (fs::exists(stdout_path)) {
        result.stdout_text = read_text(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_text(stderr_path);
    }

#if defined(_WIN32)
    result.exit_code = raw_exit_code;
#else
    if (raw_exit_code != -1 && WIFEXITED(raw_exit_code)) {
        result.exit_code = WEXITSTATUS(raw_exit_code);
    } else {
        result.exit_code = raw_exit_code;
    }
#endif
    return result;
}

void write_synthetic_report_table_for_layout_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "10", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2799: synthetic report/label layout settings fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2799: synthetic report/label layout settings fixture should mark deleted objects");
}

void write_synthetic_report_table_for_deleted_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#2799: deleted settings fixture should mark report settings deleted");
}

void write_synthetic_report_table_for_stable_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#2799: stable settings fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#2799: stable settings fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_deleted_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_settings_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#2799: stable deleted settings fixture should seed a deleted settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#2799: stable deleted settings fixture should preserve the deleted settings state");
}

void run_live_settings_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_settings_json(asset_path);

    const auto settings_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "settings-guid", "--json"},
        temp_root);

    if (settings_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected settings stdout:\n"
                  << settings_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected settings stderr:\n"
                  << settings_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(settings_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(settings_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(settings_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected-settings availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should expose settings selection kind");
    expect_contains(settings_process.stdout_text, "\"settingCount\": 6",
                    issue_prefix + " should preserve live setting counts");
    expect_contains(settings_process.stdout_text, "\"deletedSettingCount\": 0",
                    issue_prefix + " should preserve deleted setting counts");
    expect_contains(settings_process.stdout_text, "\"pageSetupAvailable\": true",
                    issue_prefix + " should preserve page setup availability");
    expect_contains(settings_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should expose live preview availability");
    expect_contains(settings_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsRight\": 5200",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(settings_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should preserve deleted preview availability");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    issue_prefix + " should preserve deleted preview right bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    issue_prefix + " should preserve deleted preview widths");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    issue_prefix + " should not advertise selected-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSection\": null",
                    issue_prefix + " should serialize null selected sections");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise containing-object-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null containing-object sections");
    expect_contains(settings_process.stdout_text,
                    "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"0\"",
                    issue_prefix + " should expose memo-line setting provenance");
    expect_contains(settings_process.stdout_text,
                    "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"1\"",
                    issue_prefix + " should expose later memo-line setting provenance");
    expect_contains(settings_process.stdout_text,
                    "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"10\"",
                    issue_prefix + " should expose direct setting provenance");
}

void run_deleted_settings_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_deleted_settings_json(asset_path);

    const auto settings_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "deleted-settings-guid", "--json"},
        temp_root);

    if (settings_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected deleted settings stdout:\n"
                  << settings_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected deleted settings stderr:\n"
                  << settings_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(settings_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(settings_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(settings_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected-settings availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should expose settings selection kind");
    expect_contains(settings_process.stdout_text, "\"settingCount\": 0",
                    issue_prefix + " should not expose live settings");
    expect_contains(settings_process.stdout_text, "\"deletedSettingCount\": 6",
                    issue_prefix + " should expose deleted setting counts");
    expect_contains(settings_process.stdout_text, "\"pageSetupAvailable\": true",
                    issue_prefix + " should expose effective deleted-root page setup");
    expect_contains(settings_process.stdout_text, "\"orientationAvailable\": true",
                    issue_prefix + " should expose effective deleted-root orientation availability");
    expect_contains(settings_process.stdout_text, "\"orientationCode\": 0",
                    issue_prefix + " should expose the effective deleted-root orientation");
    expect_contains(settings_process.stdout_text, "\"paperSizeAvailable\": true",
                    issue_prefix + " should expose effective deleted-root paper-size availability");
    expect_contains(settings_process.stdout_text, "\"paperSizeCode\": 1",
                    issue_prefix + " should expose the effective deleted-root paper size");
    expect_contains(settings_process.stdout_text, "\"topMarginAvailable\": true",
                    issue_prefix + " should expose effective deleted-root top-margin availability");
    expect_contains(settings_process.stdout_text, "\"topMargin\": 10",
                    issue_prefix + " should expose the effective deleted-root top margin");
    expect_contains(settings_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve live preview availability");
    expect_contains(settings_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsRight\": 5200",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(settings_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should preserve deleted preview availability");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    issue_prefix + " should preserve deleted preview right bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    issue_prefix + " should preserve deleted preview widths");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    issue_prefix + " should not advertise selected-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSection\": null",
                    issue_prefix + " should serialize null selected sections");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise containing-object-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null containing-object sections");
    expect_contains_in_order(
        settings_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
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
        issue_prefix + " should expose selected deleted-setting provenance");
}

void test_studio_host_json_preserves_selected_settings_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_settings_stable_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_live_settings_selection(
        studio_host_path,
        temp_root,
        "selected_settings_stable.frx",
        "report",
        "#2799: stable live report settings selection");
    run_live_settings_selection(
        studio_host_path,
        temp_root,
        "selected_settings_stable.lbx",
        "label",
        "#2799: stable live label settings selection");
    run_deleted_settings_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_settings_stable.frx",
        "report",
        "#2799: stable deleted report settings selection");
    run_deleted_settings_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_settings_stable.lbx",
        "label",
        "#2799: stable deleted label settings selection");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_settings_stable <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_settings_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
